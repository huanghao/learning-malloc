#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "print_float_sem.h"

float make_qnan_f() {
  uint32_t bits = 0x7FC00001;  // E=0xFF, M=1<<22 (最高位1) + payload 1
  float f;
  memcpy(&f, &bits, sizeof bits);
  return f;
}

float make_snan_f() {
  uint32_t bits = 0x7F800001;  // E=0xFF, M=1 (最高位0，但其它位非0)
  float f;
  memcpy(&f, &bits, sizeof bits);
  return f;
}

int main() {
  float qf = make_qnan_f(), sf = make_snan_f();

  printf("qNaN float: %08X,  sNaN float: %08X\n", *(uint32_t*)&qf,
         *(uint32_t*)&sf);
  // 验证比较结果
  printf("qf==qf: %d, sf==sf: %d\n", qf == qf, sf == sf);

  printf("qNaN float: %f\n====\n", qf);
  print_float_sem(qf);
  qf += 1.0;

  printf("sNaN float: %f\n====\n", sf);
  print_float_sem(sf);
  sf += 1.0;
  return 0;
}
