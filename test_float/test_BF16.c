#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#if defined(__x86_64__) || defined(_M_X64)
#include <cpuid.h>
#endif
#if defined(__aarch64__)
#include <arm_neon.h>
#endif

// 检查CPU是否支持BF16
int cpu_supports_bf16() {
#if defined(__x86_64__) || defined(_M_X64)
  unsigned int eax, ebx, ecx, edx;
  if (__get_cpuid_count(7, 1, &eax, &ebx, &ecx, &edx)) {
    return (ecx & (1 << 5)) != 0;
  }
  return 0;
#elif defined(__aarch64__)
  FILE *f = fopen("/proc/cpuinfo", "r");
  if (!f) return 0;
  char line[512];
  while (fgets(line, sizeof(line), f)) {
    if (strstr(line, "Features") && strstr(line, "bf16")) {
      fclose(f);
      return 1;
    }
  }
  fclose(f);
  return 0;
#else
  return 0;  // TODO: add other platform support
#endif
}

double test_bf16(int N) {
  clock_t t1, t2;

#if defined(__aarch64__) && defined(__ARM_FEATURE_BF16_SCALAR_ARITHMETIC)
  printf("CPU supports BF16 (ARMv8.6+, asimdhp/bf16) scalar\n");
  // ARMv8.6+ 硬件BF16指令（标量）
  volatile __bf16 a = 1.1, b = 2.2;
  volatile __bf16 cb2 = 0;
  volatile float cb = 0;
  t1 = clock();
  for (int i = 0; i < N; ++i) {
    __bf16 res = a * b;
    cb += (float)res;
    cb2 += res;
    if (i % 1000000 == 0) {
      printf("n=%d, result=%f (hardware __bf16), fp32=%f\n", i, (float)cb2, cb);
    }
  }
  t2 = clock();
  printf("BF16: %f seconds, result=%f (hardware __bf16), fp32=%f\n",
         (float)(t2 - t1) / CLOCKS_PER_SEC, (float)cb2, cb);
  return cb;
#elif defined(__aarch64__) && defined(__ARM_FEATURE_BF16_VECTOR_ARITHMETIC)
  printf("CPU supports BF16 (ARMv8.6+, asimdhp/bf16) vector\n");
  // ARMv8.6+ 硬件BF16指令（向量）
  t1 = clock();
  volatile double cb2 = 0;
  float32x4_t va = vdupq_n_f32(a);
  float32x4_t vb = vdupq_n_f32(b);
  for (int i = 0; i < N / 4; ++i) {
    bfloat16x4_t vab = vcvt_bf16_f32(va);
    bfloat16x4_t vbb = vcvt_bf16_f32(vb);
    bfloat16x4_t vres = vmul_bf16(vab, vbb);
    float32x4_t vresf = vcvt_f32_bf16(vres);
    cb2 += vgetq_lane_f32(vresf, 0) + vgetq_lane_f32(vresf, 1) +
           vgetq_lane_f32(vresf, 2) + vgetq_lane_f32(vresf, 3);
  }
  t2 = clock();
  printf("BF16: %f seconds, result=%f (hardware NEON)\n",
         (double)(t2 - t1) / CLOCKS_PER_SEC, cb2);
  return cb2;
#endif
}

double test_float32(int N) {
  volatile float a = 1.1f, b = 2.2f, c = 0;
  clock_t t1, t2;

  // float32
  t1 = clock();
  for (int i = 0; i < N; ++i) {
    c += a * b;
    if (i % 1000000 == 0) {
      printf("n=%d, result=%f\n", i, c);
    }
  }
  t2 = clock();
  printf("FP32: %f seconds, result=%f\n", (double)(t2 - t1) / CLOCKS_PER_SEC,
         c);
  return c;
}

double test_float64(int N) {
  volatile double a = 1.1, b = 2.2, c = 0;
  clock_t t1, t2;
  t1 = clock();
  for (int i = 0; i < N; ++i) {
    c += a * b;
    if (i % 1000000 == 0) {
      printf("n=%d, result=%f\n", i, c);
    }
  }
  t2 = clock();
  printf("FP64: %f seconds, result=%f\n", (double)(t2 - t1) / CLOCKS_PER_SEC,
         c);
  return c;
}

int main() {
  int has_bf16 = cpu_supports_bf16();
  if (has_bf16) {
#if defined(__aarch64__)
    printf("CPU supports BF16 (ARM, asimdhp/bf16)\n");
#else
    printf("CPU supports BF16 (AVX512 BF16)\n");
#endif
  } else {
    printf("CPU does NOT support BF16\n");
  }

  const int N = 10000000;

  // bf16
  printf("---- Testing BF16 ----\n");
  double res_bf16 = test_bf16(N);

  // float32
  printf("---- Testing float ----\n");
  double res_float32 = test_float32(N);

  // double
  printf("---- Testing double ----\n");
  double res_float64 = test_float64(N);

  double err_rate_bf16 = fabs((res_bf16 - res_float64) / res_float64);
  double err_rate_float32 = fabs((res_float32 - res_float64) / res_float64);
  printf("N = %d\n", N);
  printf("%10s %20s %20s\n", "type", "result", "err rate");
  printf("%10s %20f %20f\n", "bf16", res_bf16, err_rate_bf16);
  printf("%10s %20f %20f\n", "float32", res_float32, err_rate_float32);
  printf("%10s %20f %20f\n", "float64", res_float64, 0.0);

  return 0;
}
