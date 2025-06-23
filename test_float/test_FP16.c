#include <stdio.h>
#include <time.h>
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || \
    defined(_M_IX86)
#include <cpuid.h>
#endif

// 检查CPU是否支持FP16（F16C指令集，x86平台；ARMv8.2-A及以上，arm64平台）
int cpu_supports_fp16() {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || \
    defined(_M_IX86)
  unsigned int eax, ebx, ecx, edx;
  if (__get_cpuid_max(0, NULL) < 1) return 0;
  __cpuid_count(1, 0, eax, ebx, ecx, edx);
  // F16C is bit 29 of ECX
  return (ecx & (1 << 29)) != 0;
#elif defined(__aarch64__) || defined(__arm__)
// ARM: 检查编译器是否定义了FP16相关宏
#if defined(__ARM_FEATURE_FP16_VECTOR_ARITHMETIC) || \
    defined(__ARM_FEATURE_FP16_SCALAR_ARITHMETIC)
  return 1;
#else
  return 0;
#endif
#else
  // 其他平台可扩展
  return 0;
#endif
}

int main() {
  // 检查CPU支持
  if (cpu_supports_fp16()) {
    printf("CPU supports FP16 (F16C)\n");
  } else {
    printf("CPU does NOT support FP16 (F16C)\n");
  }

  // 检查编译器是否支持_Float16
#ifdef __FLT16_MANT_DIG__
  const int N = 10000000;
  volatile_Float16 a = 1.1, b = 2.2, c = 0;
  float fa = 1.1f, fb = 2.2f, fc = 0;
  clock_t t1, t2;

  t1 = clock();
  for (int i = 0; i < N; ++i) c += a * b;
  t2 = clock();
  printf("FP16: %f seconds, result=%f\n", (double)(t2 - t1) / CLOCKS_PER_SEC,
         (float)c);

  t1 = clock();
  for (int i = 0; i < N; ++i) fc += fa * fb;
  t2 = clock();
  printf("FP32: %f seconds, result=%f\n", (double)(t2 - t1) / CLOCKS_PER_SEC,
         fc);
#else
  printf("Your compiler does not support _Float16.\n");
#endif
  return 0;
}
