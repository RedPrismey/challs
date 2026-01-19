#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char BANNER[] =
    "=================================================================\n"
    "  _________      /\\/\\              .__.__       .__\n"
    "  \\_   ___ \\  /\\ \\ \\ \\   _______  _|__|  |      |  |__   ______\n"
    "  /    \\  \\/  \\/  \\ \\ \\_/ __ \\  \\/ /  |  |      |  |  \\ /  ___/\n"
    "  \\     \\____ /\\   \\ \\ \\  ___/\\   /|  |  |__    |   Y  \\\\___ \\\n"
    "   \\______  / \\/    \\ \\ \\___  >\\_/ |__|____/ /\\ |___|  /____  >\n"
    "          \\/         \\/\\/   \\/               \\/      \\/     \\/\n"
    "                         evil manager v0.0.2\n"
    "=================================================================\n"
    "               Your evil plans are now centralized\n"
    " (Le code est encore en alpha mais au moins il devrait être sûr)\n";

void boot() {
  setvbuf(stdin, NULL, _IONBF, 0);
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);
}

void gadget() {
  asm("pop %rdi");
  asm("ret");
}

int main(int argc, char *argv[]) {
  boot();

  char buf[20];

  puts(BANNER);
  printf("Entrez votre nom d'utilisateur\n> ");

  read(0, buf, 0x100);
  printf("Entrez le mot de passe pour %s\n> ", buf);

  read(0, buf, 0x100);

  return EXIT_SUCCESS;
}
