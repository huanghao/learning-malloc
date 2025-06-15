#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

int ready = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *consumer(void *arg) {
  (void)arg;  // 避免未使用参数警告
  pthread_mutex_lock(&mutex);

  // ❌ 错误写法
  // if (ready == 0) {
  //     pthread_cond_wait(&cond, &mutex);
  // }

  // ✅ 正确写法
  while (ready == 0) {  // 用while，防止虚假唤醒
    printf("Consumer waiting...\n");
    pthread_cond_wait(&cond, &mutex);
    printf("Consumer woken up! ready=%d\n", ready);
  }

  printf("Consumer: ready=%d, proceeding!\n", ready);
  pthread_mutex_unlock(&mutex);
  return NULL;
}

void *producer(void *arg) {
  (void)arg;   // 避免未使用参数警告
  sleep(0.5);  // 模拟工作

  pthread_mutex_lock(&mutex);
  ready = 1;
  printf("Producer: set ready=1, signaling...\n");
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);
  return NULL;
}

int main() {
  pthread_t consumer_thread, producer_thread;

  printf("Starting spurious wakeup demo...\n");

  // 创建消费者和生产者线程
  pthread_create(&consumer_thread, NULL, consumer, NULL);
  pthread_create(&producer_thread, NULL, producer, NULL);

  // 等待两个线程完成
  pthread_join(consumer_thread, NULL);
  pthread_join(producer_thread, NULL);

  printf("Demo completed!\n");
  return 0;
}