#include <stdio.h>
#include <xmmintrin.h>  // SSE intrinsics

#define N 8

// 在 GNU C 或 Clang 下，你可以使用 __attribute__((aligned(16))) 强制对齐
float A[N] __attribute__((aligned(16))) = {1, 2, 3, 4, 5, 6, 7, 8};
float B[N] __attribute__((aligned(16))) = {10, 20, 30, 40, 50, 60, 70, 80};
float C[N] __attribute__((aligned(16)));

int main(void) {
  int i;
  // 每次处理 4 个 float
  for (i = 0; i < N; i += 4) {
    // 从内存加载 4 个 float 到向量寄存器
    __m128 va = _mm_load_ps(&A[i]);
    __m128 vb = _mm_load_ps(&B[i]);
    // 并行加法：一次性完成 4 次相加
    __m128 vc = _mm_add_ps(va, vb);
    // 将结果存回内存
    _mm_store_ps(&C[i], vc);
  }

  // 输出结果
  for (i = 0; i < N; ++i) {
    printf("%.0f ", C[i]);
  }
  printf("\n");
  return 0;
}
