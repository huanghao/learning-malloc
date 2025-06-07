#include <stdio.h>

int my_var __attribute__((section("mysection"))) = 123;

void my_function() __attribute__((section(".mytext")));
void my_function() { printf("Hello from my_function in .mytext section!\n"); }

int main() {
  my_function();
  return my_var;
}
