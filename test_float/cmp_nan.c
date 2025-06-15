#include <math.h>
#include <stdio.h>

int main() {
  float a = 0.0 / 0.0;
  printf("a=%f\n", a);

  printf("a==a: %d\n", a == a);  // 0
  printf("a!=a: %d\n", a != a);  // 1
  printf("a<a: %d\n", a < a);    // 0
  printf("a>a: %d\n", a > a);    // 0
  printf("a<=a: %d\n", a <= a);  // 0
  printf("a>=a: %d\n", a >= a);  // 0

  float b = sqrt(-1);
  printf("b=%f\n", b);

  printf("b==b: %d\n", b == b);  // 0
  printf("b!=b: %d\n", b != b);  // 1
  printf("b<b: %d\n", b < b);    // 0
  printf("b>b: %d\n", b > b);    // 0
  printf("b<=b: %d\n", b <= b);  // 0
  printf("b>=b: %d\n", b >= b);  // 0
  return 0;
}