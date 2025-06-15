#define _POSIX_C_SOURCE 199309L
#include <fcntl.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

void empty_function() {}

int main() {
  const int iterations = 1000000;
  struct timespec start, end;
  long i;
  double elapsed_ns;

  // 测试空函数调用
  clock_gettime(CLOCK_MONOTONIC, &start);
  for (i = 0; i < iterations; i++) {
    empty_function();
  }
  clock_gettime(CLOCK_MONOTONIC, &end);
  elapsed_ns =
      (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
  printf("空函数调用平均时间: %.2f ns\n", elapsed_ns / iterations);

  // 测试 getpid() 系统调用
  clock_gettime(CLOCK_MONOTONIC, &start);
  for (i = 0; i < iterations; i++) {
    getpid();
  }
  clock_gettime(CLOCK_MONOTONIC, &end);
  elapsed_ns =
      (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
  printf("getpid() 系统调用平均时间: %.2f ns\n", elapsed_ns / iterations);

  // 测试 gettimeofday()（通常通过 VDSO 实现）
  struct timeval tv;
  clock_gettime(CLOCK_MONOTONIC, &start);
  for (i = 0; i < iterations; i++) {
    gettimeofday(&tv, NULL);
  }
  clock_gettime(CLOCK_MONOTONIC, &end);
  elapsed_ns =
      (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
  printf("gettimeofday() VDSO调用平均时间: %.2f ns\n", elapsed_ns / iterations);

  // 测试 write() 写入一个字节到 /dev/null，真实系统调用且有 I/O
  int fd = open("/dev/null", O_WRONLY);
  if (fd == -1) {
    perror("open /dev/null");
    return 1;
  }
  char buf = 'a';
  clock_gettime(CLOCK_MONOTONIC, &start);
  for (i = 0; i < iterations; i++) {
    write(fd, &buf, 1);
  }
  clock_gettime(CLOCK_MONOTONIC, &end);
  elapsed_ns =
      (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
  printf("write() /dev/null写入平均时间: %.2f ns\n", elapsed_ns / iterations);

  close(fd);
  return 0;
}
