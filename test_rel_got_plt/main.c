// main.c
extern void external_func(int);
extern int undefined_var;

int main() {
  undefined_var++;
  external_func(undefined_var);
  return 0;
}
