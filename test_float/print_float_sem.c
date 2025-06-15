#include "print_float_sem.h"

void print_float_sem(float f) {
  uint32_t n = *(uint32_t *)&f;
  uint32_t s = (n >> 31) & 0x1;
  uint32_t e = (n >> 23) & 0xFF;
  uint32_t m = n & 0x7FFFFF;

  printf("s: %u\n", s);
  printf("e: %u (0b ", e);
  for (int i = 7; i >= 0; --i) printf("%u", (e >> i) & 1);
  printf(")\n");

  printf("m: 0x%x (0b ", m);
  for (int i = 22; i >= 0; --i) printf("%u", (m >> i) & 1);
  printf(")\n");
}