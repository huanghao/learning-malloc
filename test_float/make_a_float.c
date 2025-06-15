#include <stdint.h>
#include <stdio.h>

#include "print_float_sem.h"

float reconstruct_from_sem(float finp) {
  uint32_t n = *(uint32_t *)&finp;
  uint32_t s = (n >> 31) & 0x1;
  uint32_t e = (n >> 23) & 0xFF;
  uint32_t m = n & 0x7FFFFF;

  float f = 1.0;
  float b = 1.0;
  for (int i = 0; i < 23; ++i) {
    b /= 2;
    if ((m >> (22 - i)) & 1) {
      f += b;
    }
  }

  int exp = e - 127;
  f *= 1 << exp;
  if (s) {
    f = -f;
  }
  return f;
}

int main() {
  printf("make the float 3.14\n");
  // s 0 e 128 m 10010001111010111000010
  // 48f5c2
  uint32_t n = 0;
  n |= 128;
  n <<= 23;
  n |= 0x48f5c2;
  float *pf = (float *)&n;
  printf("%f\n", *pf);
  print_float_sem(*pf);

  printf("display the float 3.14\n");
  float f = 3.14;
  uint32_t *pn = (uint32_t *)&f;
  printf("bin: ");
  for (int i = 31; i >= 0; --i) printf("%u", (*pn >> i) & 1);
  printf("\n");

  print_float_sem(f);

  printf("reconstruct the float -3.14\n");
  float f2 = reconstruct_from_sem(-3.14);
  printf("%f\n", f2);
  print_float_sem(f2);
  return 0;
}