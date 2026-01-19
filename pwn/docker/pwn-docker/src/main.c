#include <stdio.h>
#include <stdlib.h>

int main(void) {
  setvbuf(stdin, NULL, _IONBF, 0);
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);

  system("/bin/bash");

  return 0;
}
