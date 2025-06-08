#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// 统计信息
static int malloc_count = 0;
static int free_count = 0;
static int printf_count = 0;
static int sleep_count = 0;
static int fopen_count = 0;

// 原始函数指针
static void *(*original_malloc)(size_t size) = NULL;
static void (*original_free)(void *ptr) = NULL;
static int (*original_printf)(const char *format, ...) = NULL;
static unsigned int (*original_sleep)(unsigned int seconds) = NULL;
static FILE *(*original_fopen)(const char *pathname, const char *mode) = NULL;

// 初始化原始函数指针
static void init_original_functions() {
  if (!original_malloc) {
    original_malloc = dlsym(RTLD_NEXT, "malloc");
  }
  if (!original_free) {
    original_free = dlsym(RTLD_NEXT, "free");
  }
  if (!original_printf) {
    original_printf = dlsym(RTLD_NEXT, "printf");
  }
  if (!original_sleep) {
    original_sleep = dlsym(RTLD_NEXT, "sleep");
  }
  if (!original_fopen) {
    original_fopen = dlsym(RTLD_NEXT, "fopen");
  }
}

// 拦截 malloc
void *malloc(size_t size) {
  init_original_functions();
  malloc_count++;

  void *ptr = original_malloc(size);
  fprintf(stderr, "[PRELOAD] malloc(%zu) = %p (call #%d)\n", size, ptr,
          malloc_count);
  return ptr;
}

// 拦截 free
void free(void *ptr) {
  init_original_functions();
  free_count++;

  fprintf(stderr, "[PRELOAD] free(%p) (call #%d)\n", ptr, free_count);
  original_free(ptr);
}

// 拦截 printf
int printf(const char *format, ...) {
  init_original_functions();
  printf_count++;

  // 先输出我们的信息到stderr
  fprintf(stderr, "[PRELOAD] printf called (call #%d): ", printf_count);

  // 调用原始printf
  va_list args;
  va_start(args, format);
  int result = vprintf(format, args);
  va_end(args);

  return result;
}

// 拦截 sleep
unsigned int sleep(unsigned int seconds) {
  init_original_functions();
  sleep_count++;

  fprintf(stderr,
          "[PRELOAD] sleep(%u) called (call #%d) - reducing to 1 second\n",
          seconds, sleep_count);

  // 将所有sleep调用都改为1秒，演示函数行为修改
  return original_sleep(1);
}

// 拦截 fopen
FILE *fopen(const char *pathname, const char *mode) {
  init_original_functions();
  fopen_count++;

  fprintf(stderr, "[PRELOAD] fopen(\"%s\", \"%s\") called (call #%d)\n",
          pathname, mode, fopen_count);

  return original_fopen(pathname, mode);
}

// 构造函数：库加载时调用
__attribute__((constructor)) static void preload_init() {
  fprintf(stderr, "[PRELOAD] LD_PRELOAD library loaded!\n");
  fprintf(stderr,
          "[PRELOAD] This library will intercept malloc, free, printf, sleep, "
          "and fopen calls\n");
}

// 析构函数：库卸载时调用
__attribute__((destructor)) static void preload_fini() {
  fprintf(stderr, "\n[PRELOAD] LD_PRELOAD library unloading!\n");
  fprintf(stderr, "[PRELOAD] Statistics:\n");
  fprintf(stderr, "[PRELOAD]   malloc calls: %d\n", malloc_count);
  fprintf(stderr, "[PRELOAD]   free calls: %d\n", free_count);
  fprintf(stderr, "[PRELOAD]   printf calls: %d\n", printf_count);
  fprintf(stderr, "[PRELOAD]   sleep calls: %d\n", sleep_count);
  fprintf(stderr, "[PRELOAD]   fopen calls: %d\n", fopen_count);
}