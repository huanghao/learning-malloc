#include "print_float_sem.h"

void print_float_sem(float f) {
  uint32_t n = *(uint32_t *)&f;
  uint32_t s = (n >> 31) & 0x1;
  uint32_t e = (n >> 23) & 0xFF;
  uint32_t m = n & 0x7FFFFF;

  printf("binary representation of the float %f\n", f);
  printf("%s %-8s %s\n", "s", "e", "m");
  for (int i = 31; i >= 0; --i) {
    printf("%u", (n >> i) & 1);
    if (i == 31 || i == 23) {
      printf(" ");
    }
  }
  printf("\n");
  printf("%s %-8d 0x%x\n", (s == 0) ? "+" : "-", e - 127, m);

  printf("%-3s %s %-8s %s\n", " ", " ", "base", "m");
  float f2 = 0.0;
  float b = 1.0;
  for (int i = 22; i >= 0; --i) {
    b /= 2;
    int is_one = (m >> i) & 1;

    printf("%2d: %u %8f", 23 - i, is_one, b);
    if (is_one) {
      f2 += b;
      printf(" %8f", f2);
    } else {
      printf(" ");
    }
    printf("\n");
  }
  printf("f: %f = (-1)^%d * (1 + %f) * 2^(%d-127) \n",
         (s == 0 ? 1 : -1) * (1 + f2) * (1 << (e - 127)), s, f2, e - 127);
}