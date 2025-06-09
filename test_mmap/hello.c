#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    return 1;
  }

  // 打开文件，读写权限
  int fd = open(argv[1], O_RDWR);
  if (fd == -1) {
    perror("open");
    return 1;
  }

  // 获取文件大小
  off_t filesize = lseek(fd, 0, SEEK_END);

  // 映射文件到内存
  char *map = mmap(NULL, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (map == MAP_FAILED) {
    perror("mmap");
    close(fd);
    return 1;
  }

  // 修改映射区域的内容
  memcpy(map, "Hi mmap!", 8);

  // 同步内存修改回文件
  msync(map, filesize, MS_SYNC);

  // 解除映射
  munmap(map, filesize);

  close(fd);

  printf("文件修改成功！\n");
  return 0;
}
