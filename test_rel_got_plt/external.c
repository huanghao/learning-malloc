// external.c
#include <stdio.h>

extern int var_in_level2_ext;

int undefined_var = 1;

void external_func(int a) {
  printf("Hello from external_func! %d\n",
         a + var_in_level2_ext + undefined_var);
}
