#define _POSIX_C_SOURCE 199309L
#include <pthread.h>
#include <stdio.h>
#include <time.h>

#define SWITCHES 1000000

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_a = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_b = PTHREAD_COND_INITIALIZER;
int turn = 0;  // 0 for thread A, 1 for thread B

void *thread_func_a(void *arg) {
  for (int i = 0; i < SWITCHES; i++) {
    pthread_mutex_lock(&mutex);
    while (turn != 0) pthread_cond_wait(&cond_a, &mutex);
    // printf("A\n"); // 打印会影响性能，测试时关闭
    turn = 1;
    pthread_cond_signal(&cond_b);
    pthread_mutex_unlock(&mutex);
  }
  return NULL;
}

void *thread_func_b(void *arg) {
  for (int i = 0; i < SWITCHES; i++) {
    pthread_mutex_lock(&mutex);
    while (turn != 1) pthread_cond_wait(&cond_b, &mutex);
    // printf("B\n");
    turn = 0;
    pthread_cond_signal(&cond_a);
    pthread_mutex_unlock(&mutex);
  }
  return NULL;
}

int main() {
  pthread_t thread_a, thread_b;
  struct timespec start, end;

  // 创建两个线程
  pthread_create(&thread_a, NULL, thread_func_a, NULL);
  pthread_create(&thread_b, NULL, thread_func_b, NULL);

  // 计时开始
  clock_gettime(CLOCK_MONOTONIC, &start);

  // 主线程等待两个线程完成
  pthread_join(thread_a, NULL);
  pthread_join(thread_b, NULL);

  // 计时结束
  clock_gettime(CLOCK_MONOTONIC, &end);

  long long elapsed_ns = (end.tv_sec - start.tv_sec) * 1000000000LL +
                         (end.tv_nsec - start.tv_nsec);

  // 每次线程切换的时间（2个线程之间一次切换对应一次唤醒+等待）
  // 由于每次循环包含两个切换，计算平均切换时间时除以 2 * SWITCHES
  double avg_switch_ns = (double)elapsed_ns / (2 * SWITCHES);

  printf("总时间: %lld ns\n", elapsed_ns);
  printf("平均一次线程切换时间: %.2f ns\n", avg_switch_ns);

  return 0;
}
