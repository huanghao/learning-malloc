#define _GNU_SOURCE
#include <dlfcn.h>
#include <math.h>
#include <stdio.h>

int main() {
  void *handle = dlopen("libc.so", RTLD_LAZY);
  if (!handle) {
    printf("dlopen failed\n");
    return 1;
  }

  void *addr = dlsym(handle, "printf");
  if (!addr) {
    printf("dlsym failed\n");
    dlclose(handle);
    return 1;
  }

  Dl_info info;
  if (dladdr(sin, &info)) {
    printf("function is at: %s\n", info.dli_fname);
  } else {
    printf("dladdr failed\n");
  }

  dlclose(handle);
  return 0;
}
