#include <stdio.h>
#include <stdlib.h>

long fib(void);

int main(int argc, char *argv[]) {
  long arg, res;

  if (argc != 2) {
    fprintf(stderr, "usage: %s <num>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  arg = strtol(argv[1], NULL, 0);

  asm("pushq\t%0"::"r"(arg));
  res = fib();
  asm("addq\t$8, %rsp");

  printf("fib(%ld): %ld\n", arg, res);
  
  return 0;
}