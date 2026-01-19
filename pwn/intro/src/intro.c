#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void boot() {
  setvbuf(stdin, NULL, _IONBF, 0);
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);
}

int main(int argc, char *argv[]) {
  char input[50];
  int debug_mode = 0;

  boot();

  printf("==========================================\n"
         "           __        _ _       _          \n"
         "          / /       (_) |     | |         \n"
         "         / /____   ___| |  ___| |__       \n"
         "        / / _ \\ \\ / / | | / __| '_ \\   \n"
         "     _ / /  __/\\ V /| | |_\\__ \\ | | |  \n"
         "    (_)_/ \\___| \\_/ |_|_(_)___/_| |_|   \n"
         "\n"
         "          ./evil.sh contact app           \n"
         "==========================================\n");
  printf("Entrez le message que vous voulez envoyer : ");

  read(0, input, 0x50);

  printf("\nMessage envoyé\n");

  if (debug_mode) {
    system("/bin/bash");
  }

  return EXIT_SUCCESS;
}
