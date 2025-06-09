#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int global_init_var = 42;  // data
int global_uninit_var;     // bss

void print_addresses() {
  static int static_var = 20;  // data
  int local_var = 10;          // stack
  void* heap_end = sbrk(0);

  printf("=== Memory Segment Addresses ===\n");
  printf("Function (text) address:     %p\n", (void*)print_addresses);
  printf("Initialized global (data):    %p\n", &global_init_var);
  printf("Uninitialized global (bss):  %p\n", &global_uninit_var);
  printf("Static local (data):          %p\n", &static_var);
  printf("Local var (stack):            %p\n", &local_var);
  printf("Heap end (sbrk):              %p\n", heap_end);
  printf("===============================\n");
}

int main() {
  print_addresses();

  // 动态分配内存演示
  printf("Allocating 1MB on heap...\n");
  void* p = malloc(1024 * 1024);
  if (!p) {
    perror("malloc");
    return 1;
  }

  print_addresses();

  free(p);
  printf("Freed allocated memory.\n");

  print_addresses();

  printf("Press Enter to exit...\n");
  getchar();  // 等待输入，阻塞进程
  return 0;
}
