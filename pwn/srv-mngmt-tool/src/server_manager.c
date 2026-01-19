#include <stdio.h>
#include <stdlib.h>

void boot() {
  setvbuf(stdin, NULL, _IONBF, 0);
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);
}

void system_stats() {
  printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
         "-------------[System statistics]-------------\n"
         "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
         "  cpu:•••••••••••••••••••••••21%%\n"
         "  ram:•••••••••••••••••••••••18%%\n"
         "  stockage:••••••••••••••••••37%%\n"
         "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");
}

void feedback() {
  char feedback[64];

  printf(
      "\nEntrez le retour que vous voulez nous faire, et nous le prendrons en "
      "compte pour la prochaine release: ");
  scanf("%s", feedback);
}

void clear_stdin() {
  int c;
  while ((c = getchar()) != '\n' && c != EOF) {
  }
}

void remote_admin() { system("/bin/sh"); }

int main() {
  int menu = -1;

  boot();

  printf(
      "=================================================================\n"
      "  _________      /\\/\\              .__.__       .__\n"
      "  \\_   ___ \\  /\\ \\ \\ \\   _______  _|__|  |      |  |__   ______\n"
      "  /    \\  \\/  \\/  \\ \\ \\_/ __ \\  \\/ /  |  |      |  |  \\ /  "
      "___/\n"
      "  \\     \\____ /\\   \\ \\ \\  ___/\\   /|  |  |__    |   Y  \\\\___ "
      "\\\n"
      "   \\______  / \\/    \\ \\ \\___  >\\_/ |__|____/ /\\ |___|  /____  >\n"
      "          \\/         \\/\\/   \\/               \\/      \\/     \\/\n"
      "                  C:\\\\evil.hs remote admin tool\n"
      "=================================================================\n"
      "Bienvenue sur l'outil de management ./insa.sh. Notre outil est encore "
      "en développement, donc n'hésitez pas à nous faire un retour.\n");

  while (menu) {
    printf("Menu\n"
           "  0: quitter\n"
           "  1: statistiques du système\n"
           "  2: faire un retour\n"
           "Que voulez vous faire: ");

    if (scanf("%i", &menu) != 1) {
      clear_stdin();
    }

    if (menu == 1) {
      system_stats();
    } else if (menu == 2) {
      feedback();
    } else if (menu != 0) {
      printf("Error option unrecognized\n"
             " \\___{ function : main\n"
             "      \\___{ address : %p\n\n",
             main);
    }
  }

  return EXIT_SUCCESS;
}
