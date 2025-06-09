#include <stdio.h>

// 声明 foo 函数为弱符号，提供默认实现
void foo() __attribute__((weak));

void foo() { printf("This is the weak default foo()\n"); }

int main() {
  foo();
  return 0;
}
