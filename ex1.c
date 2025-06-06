#include <stdio.h>
#include <unistd.h>

int global_init_var = 42;  // data
int global_uninit_var;     // bss

void foo() {
  int local_var = 10;          // stack
  static int static_var = 20;  // data
  void* heap_end = sbrk(0);

  printf("Function foo (text) address: %p\n", (void*)foo);
  printf("Initialized global var (data): %p\n", &global_init_var);
  printf("Uninitialized global var (bss): %p\n", &global_uninit_var);
  printf("Static var (data): %p\n", &static_var);
  printf("Local var (stack): %p\n", &local_var);
  printf("Heap end (sbrk): %p\n", heap_end);
}

int main() {
  foo();
  return 0;
}
