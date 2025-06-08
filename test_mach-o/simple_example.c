#include <stdio.h>
#include <stdlib.h>

// 全局变量（存储在DATA段）
int global_var = 42;
int uninitialized_var;

// 常量（存储在TEXT段）
const char *greeting = "Hello, Mach-O!";

// 函数（存储在TEXT段）
void hello_function() { printf("This is a function in the TEXT segment\n"); }

int add_numbers(int a, int b) { return a + b; }

int main() {
  // 局部变量（存储在栈上）
  int local_var = 100;

  printf("%s\n", greeting);
  printf("Global variable: %d\n", global_var);
  printf("Local variable: %d\n", local_var);
  printf("Uninitialized variable: %d\n", uninitialized_var);

  hello_function();

  int result = add_numbers(10, 20);
  printf("10 + 20 = %d\n", result);

  return 0;
}