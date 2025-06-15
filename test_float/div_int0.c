#include <stdio.h>

int get_b(int a) { return 43 - a * 2 + 42 + 1; }

int div_int(int a, int b) { return a / b; }

int main() {
  int a = 43;
  volatile int b = get_b(a);
  printf("b=%d\n", b);

  int c = div_int(a, b);
  printf("c=%d\n", c);

  return 0;
}