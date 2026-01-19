#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

char BANNER[] =
    "=================================================================\n"
    "  _________      /\\/\\              .__.__       .__\n"
    "  \\_   ___ \\  /\\ \\ \\ \\   _______  _|__|  |      |  |__   ______\n"
    "  /    \\  \\/  \\/  \\ \\ \\_/ __ \\  \\/ /  |  |      |  |  \\ /  ___/\n"
    "  \\     \\____ /\\   \\ \\ \\  ___/\\   /|  |  |__    |   Y  \\\\___ \\\n"
    "   \\______  / \\/    \\ \\ \\___  >\\_/ |__|____/ /\\ |___|  /____  >\n"
    "          \\/         \\/\\/   \\/               \\/      \\/     \\/\n"
    "                           x64 asm runner\n"
    "=================================================================";

void boot() {
  setvbuf(stdin, NULL, _IONBF, 0);
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);
}

int main() {
  void *shellcode;
  shellcode = mmap(NULL, 0x100, PROT_READ | PROT_WRITE | PROT_EXEC,
                   MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

  boot();

  printf("%s\nEntrez un shellcode de moins de 10 octets : ", BANNER);

  read(0, shellcode, 10);

  ((void (*)())shellcode)();

  return EXIT_SUCCESS;
}
