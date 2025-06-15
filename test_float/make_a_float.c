#include <stdint.h>
#include <stdio.h>

#include "print_float_sem.h"

int main() {
  // s 0 e 128 m 10010001111010111000010
  // 48f5c2
  uint32_t n = 0;
  n |= 128;
  n <<= 23;
  n |= 0x48f5c2;
  float *pf = (float *)&n;
  printf("%f\n", *pf);
  print_float_sem(*pf);
  print_float_sem(3.14);
  print_float_sem(-3.14);
  return 0;
}