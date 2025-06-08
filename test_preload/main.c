#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
  printf("=== LD_PRELOAD Demo Program ===\n");

  // 测试 malloc
  printf("\n1. Testing malloc:\n");
  void *ptr1 = malloc(100);
  printf("Allocated 100 bytes at: %p\n", ptr1);

  void *ptr2 = malloc(200);
  printf("Allocated 200 bytes at: %p\n", ptr2);

  // 测试 free
  printf("\n2. Testing free:\n");
  free(ptr1);
  printf("Freed first allocation\n");
  free(ptr2);
  printf("Freed second allocation\n");

  // 测试 printf
  printf("\n3. Testing printf:\n");
  printf("This is a test string\n");
  printf("Number: %d, String: %s\n", 42, "hello");

  // 测试 sleep
  printf("\n4. Testing sleep:\n");
  printf("Sleeping for 2 seconds...\n");
  sleep(2);
  printf("Wake up!\n");

  // 测试 fopen/fclose
  printf("\n5. Testing file operations:\n");
  FILE *fp = fopen("/tmp/test.txt", "w");
  if (fp) {
    fprintf(fp, "Hello World\n");
    fclose(fp);
    printf("File operations completed\n");
  }

  printf("\n=== Program finished ===\n");
  return 0;
}