#include <arm_neon.h>
#include <stdio.h>

#define N 8

float A[N] = {1, 2, 3, 4, 5, 6, 7, 8};
float B[N] = {10, 20, 30, 40, 50, 60, 70, 80};
float C[N];

int main(void) {
  for (int i = 0; i < N; i += 4) {
    // vld1q_f32 从内存加载 4 个 float
    float32x4_t va = vld1q_f32(&A[i]);
    float32x4_t vb = vld1q_f32(&B[i]);
    // vaddq_f32 并行做 4 次加法
    float32x4_t vc = vaddq_f32(va, vb);
    // vst1q_f32 存回内存
    vst1q_f32(&C[i], vc);
  }

  for (int i = 0; i < N; i++) {
    printf("%.0f ", C[i]);
  }
  printf("\n");
  return 0;
}
