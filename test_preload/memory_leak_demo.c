#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// 模拟一个有内存泄漏的函数
void function_with_leak() {
  printf("Allocating memory in function_with_leak...\n");

  // 分配内存但不释放（内存泄漏）
  char *leak1 = malloc(100);
  strcpy(leak1, "This memory will leak!");

  char *leak2 = malloc(200);
  sprintf(leak2, "Another leak of %d bytes", 200);

  // 只释放一个
  free(leak1);
  // leak2 没有被释放！
}

// 正常的内存使用
void normal_memory_usage() {
  printf("Normal memory allocation and deallocation...\n");

  char *ptr1 = malloc(150);
  char *ptr2 = calloc(10, 20);  // 10 * 20 = 200 bytes

  strcpy(ptr1, "This will be properly freed");
  memset(ptr2, 0x42, 200);

  free(ptr1);
  free(ptr2);
}

// 测试 realloc
void test_realloc() {
  printf("Testing realloc...\n");

  char *ptr = malloc(50);
  strcpy(ptr, "Initial string");

  ptr = realloc(ptr, 100);
  strcat(ptr, " - expanded");

  ptr = realloc(ptr, 200);
  strcat(ptr, " - expanded again");

  printf("Final string: %s\n", ptr);
  free(ptr);
}

// 故意的double free（应该被检测到）
void test_double_free() {
  printf("Testing double free detection...\n");

  char *ptr = malloc(50);
  strcpy(ptr, "This will cause double free");

  free(ptr);
  // 注意：下面这行会导致未定义行为，仅用于演示
  // free(ptr);  // double free - 暂时注释掉以免程序崩溃
}

int main() {
  printf("=== Memory Leak Detection Demo ===\n");

  // 正常使用
  normal_memory_usage();

  // 有泄漏的函数
  function_with_leak();

  // 测试 realloc
  test_realloc();

  // 测试 double free 检测
  test_double_free();

  // 再次分配一些内存但不释放
  printf("Allocating more memory that will leak...\n");
  void *big_leak = malloc(1024);
  void *small_leak = malloc(64);

  // 使用一下这些内存
  memset(big_leak, 0x55, 1024);
  memset(small_leak, 0xAA, 64);

  printf("Program ending (some memory will leak)...\n");
  return 0;
}