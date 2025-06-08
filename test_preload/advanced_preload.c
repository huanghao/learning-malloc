#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

// 内存分配记录结构
typedef struct alloc_record {
  void *ptr;
  size_t size;
  struct timeval alloc_time;
  const char *file;
  int line;
  struct alloc_record *next;
} alloc_record_t;

// 全局统计信息
static alloc_record_t *alloc_list = NULL;
static int total_allocations = 0;
static int total_frees = 0;
static size_t total_allocated = 0;
static size_t current_allocated = 0;
static size_t peak_allocated = 0;

// 原始函数指针
static void *(*original_malloc)(size_t size) = NULL;
static void *(*original_calloc)(size_t nmemb, size_t size) = NULL;
static void *(*original_realloc)(void *ptr, size_t size) = NULL;
static void (*original_free)(void *ptr) = NULL;

// 互斥锁（简单的忙等待）
static volatile int lock = 0;

static void simple_lock() {
  while (__sync_lock_test_and_set(&lock, 1)) {
    usleep(1);
  }
}

static void simple_unlock() { __sync_lock_release(&lock); }

// 初始化原始函数指针
static void init_original_functions() {
  if (!original_malloc) {
    original_malloc = dlsym(RTLD_NEXT, "malloc");
    original_calloc = dlsym(RTLD_NEXT, "calloc");
    original_realloc = dlsym(RTLD_NEXT, "realloc");
    original_free = dlsym(RTLD_NEXT, "free");
  }
}

// 添加分配记录
static void add_alloc_record(void *ptr, size_t size) {
  if (!ptr) return;

  alloc_record_t *record = original_malloc(sizeof(alloc_record_t));
  if (!record) return;

  record->ptr = ptr;
  record->size = size;
  gettimeofday(&record->alloc_time, NULL);
  record->file = "unknown";
  record->line = 0;
  record->next = alloc_list;
  alloc_list = record;

  total_allocations++;
  total_allocated += size;
  current_allocated += size;
  if (current_allocated > peak_allocated) {
    peak_allocated = current_allocated;
  }
}

// 移除分配记录
static void remove_alloc_record(void *ptr) {
  if (!ptr) return;

  alloc_record_t **current = &alloc_list;
  while (*current) {
    if ((*current)->ptr == ptr) {
      alloc_record_t *to_free = *current;
      current_allocated -= to_free->size;
      *current = to_free->next;
      original_free(to_free);
      total_frees++;
      return;
    }
    current = &(*current)->next;
  }

  // 如果找不到记录，说明可能是double free或者free未分配的内存
  fprintf(stderr,
          "[PRELOAD] WARNING: Attempting to free untracked pointer: %p\n", ptr);
}

// 拦截 malloc
void *malloc(size_t size) {
  init_original_functions();

  void *ptr = original_malloc(size);

  simple_lock();
  add_alloc_record(ptr, size);
  simple_unlock();

  fprintf(stderr, "[PRELOAD] malloc(%zu) = %p [current: %zu bytes]\n", size,
          ptr, current_allocated);
  return ptr;
}

// 拦截 calloc
void *calloc(size_t nmemb, size_t size) {
  init_original_functions();

  void *ptr = original_calloc(nmemb, size);

  simple_lock();
  add_alloc_record(ptr, nmemb * size);
  simple_unlock();

  fprintf(stderr, "[PRELOAD] calloc(%zu, %zu) = %p [current: %zu bytes]\n",
          nmemb, size, ptr, current_allocated);
  return ptr;
}

// 拦截 realloc
void *realloc(void *ptr, size_t size) {
  init_original_functions();

  simple_lock();
  if (ptr) {
    remove_alloc_record(ptr);
  }
  simple_unlock();

  void *new_ptr = original_realloc(ptr, size);

  simple_lock();
  add_alloc_record(new_ptr, size);
  simple_unlock();

  fprintf(stderr, "[PRELOAD] realloc(%p, %zu) = %p [current: %zu bytes]\n", ptr,
          size, new_ptr, current_allocated);
  return new_ptr;
}

// 拦截 free
void free(void *ptr) {
  init_original_functions();

  if (!ptr) {
    fprintf(stderr, "[PRELOAD] free(NULL) - ignored\n");
    return;
  }

  simple_lock();
  remove_alloc_record(ptr);
  simple_unlock();

  fprintf(stderr, "[PRELOAD] free(%p) [current: %zu bytes]\n", ptr,
          current_allocated);
  original_free(ptr);
}

// 显示内存统计信息
static void show_memory_stats() {
  fprintf(stderr, "\n[PRELOAD] === Memory Statistics ===\n");
  fprintf(stderr, "[PRELOAD] Total allocations: %d\n", total_allocations);
  fprintf(stderr, "[PRELOAD] Total frees: %d\n", total_frees);
  fprintf(stderr, "[PRELOAD] Memory leaks: %d allocations\n",
          total_allocations - total_frees);
  fprintf(stderr, "[PRELOAD] Total allocated: %zu bytes\n", total_allocated);
  fprintf(stderr, "[PRELOAD] Peak memory usage: %zu bytes\n", peak_allocated);
  fprintf(stderr, "[PRELOAD] Current allocated: %zu bytes\n",
          current_allocated);
}

// 显示内存泄漏详情
static void show_memory_leaks() {
  if (!alloc_list) {
    fprintf(stderr, "[PRELOAD] No memory leaks detected!\n");
    return;
  }

  fprintf(stderr, "\n[PRELOAD] === Memory Leaks Detected ===\n");
  alloc_record_t *current = alloc_list;
  int leak_count = 0;
  size_t leak_size = 0;

  while (current) {
    struct timeval now;
    gettimeofday(&now, NULL);
    double leak_age = (now.tv_sec - current->alloc_time.tv_sec) +
                      (now.tv_usec - current->alloc_time.tv_usec) / 1000000.0;

    fprintf(stderr, "[PRELOAD] LEAK: %p (%zu bytes, %.2f seconds old)\n",
            current->ptr, current->size, leak_age);
    leak_count++;
    leak_size += current->size;
    current = current->next;
  }

  fprintf(stderr, "[PRELOAD] Total leaks: %d allocations, %zu bytes\n",
          leak_count, leak_size);
}

// 构造函数
__attribute__((constructor)) static void advanced_preload_init() {
  fprintf(stderr, "[PRELOAD] Advanced Memory Tracker loaded!\n");
  fprintf(stderr,
          "[PRELOAD] Tracking malloc, calloc, realloc, and free calls\n");
}

// 析构函数
__attribute__((destructor)) static void advanced_preload_fini() {
  fprintf(stderr, "\n[PRELOAD] Advanced Memory Tracker unloading!\n");
  show_memory_stats();
  show_memory_leaks();

  // 清理记录链表
  while (alloc_list) {
    alloc_record_t *next = alloc_list->next;
    original_free(alloc_list);
    alloc_list = next;
  }
}