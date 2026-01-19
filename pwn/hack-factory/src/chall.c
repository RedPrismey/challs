#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char BANNER[] =
    "=================================================================\n"
    "  _________      /\\/\\              .__.__       .__\n"
    "  \\_   ___ \\  /\\ \\ \\ \\   _______  _|__|  |      |  |__   ______\n"
    "  /    \\  \\/  \\/  \\ \\ \\_/ __ \\  \\/ /  |  |      |  |  \\ /  ___/\n"
    "  \\     \\____ /\\   \\ \\ \\  ___/\\   /|  |  |__    |   Y  \\\\___ \\\n"
    "   \\______  / \\/    \\ \\ \\___  >\\_/ |__|____/ /\\ |___|  /____  >\n"
    "          \\/         \\/\\/   \\/               \\/      \\/     \\/\n"
    "                      hack factory v0.7.8\n"
    "=================================================================";

char MENU[] = "\nQue voulez vous faire :\n"
              "1. Configurer un nouveau pc\n"
              "2. Lancer l'attaque avec le pc\n"
              "3. Écrire un message maléfique\n"
              "4. Afficher le message maléfique\n"
              "5. Admin\n"
              "0. Exit";

struct Computer {
  char cpu[8];
  char os[16];
  char hdd_capacity[8];
  void (*infect_network)();
};

struct EvilMsg {
  char content[40];
};

void boot() {
  setvbuf(stdin, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);
  setvbuf(stdout, NULL, _IONBF, 0);
}

void win() { system("/bin/bash"); }

void admin() {
  FILE *f;
  char password[40];
  char input[sizeof(password)];
  int len;

  printf("Entrez le mot de passe admin : ");
  len = read(0, input, sizeof(input));
  input[len - 1] = '\0';

  f = fopen("flag.txt", "r");
  len = read(f->_fileno, password, sizeof(password));
  password[len - 1] = '\0';

  if (strncmp(input, password, len)) {
    puts("Mauvais mot de passe");
    return;
  } else {
    win();
  }
}

void hackingue() { puts("le hackingue"); }

struct Computer *new_computer(char *cpu, char *os, char *hdd_capacity) {
  struct Computer *c = malloc(sizeof(struct Computer));

  strncpy(c->cpu, cpu, sizeof(c->cpu) - 1);
  c->cpu[sizeof(c->cpu) - 1] = '\0';

  strncpy(c->os, os, sizeof(c->os) - 1);
  c->os[sizeof(c->os) - 1] = '\0';

  strncpy(c->hdd_capacity, hdd_capacity, sizeof(c->hdd_capacity) - 1);
  c->hdd_capacity[sizeof(c->hdd_capacity) - 1] = '\0';

  c->infect_network = hackingue;

  return c;
}

struct Computer *init_computer() {
  struct Computer *c = NULL;

  char cpu[sizeof(c->cpu)];
  char os[sizeof(c->os)];
  char hdd_capacity[sizeof(c->hdd_capacity)];

  int len;

  printf("Quel est le processeur du pc : ");
  len = read(0, cpu, sizeof(c->cpu));
  cpu[len - 1] = '\0';

  printf("Quel est l'os du pc : ");
  len = read(0, os, sizeof(c->os));
  os[len - 1] = '\0';

  printf("Quel est la capacité du disque dur du pc : ");
  len = read(0, hdd_capacity, sizeof(c->hdd_capacity));
  hdd_capacity[len - 1] = '\0';

  c = new_computer(cpu, os, hdd_capacity);

  return c;
}

int main(void) {
  boot();

  struct Computer *computer;
  struct EvilMsg *message;

  char menu = -1, t;

  puts(BANNER);
  puts("> Étant donné le succès de l'opération d'infection du réseau de "
       "./insa.sh, C:\\\\evil.hs a décidé de reproduire l'opération à plus "
       "grande échelle.\n"
       "> Nous inaugurons donc la hack factory, dans laquelle vous pourrez "
       "construire des ordinateurs pour aller semer le chaos dans d'autres "
       "organisations !");

  while (1) {
    puts(MENU);
    printf("> ");

    menu = getc(stdin);
    while ((t = getchar()) != '\n' && t != EOF) {
    }

    switch (menu) {
    case '1':
      computer = init_computer();
      break;

    case '2':
      if (computer) {
        computer->infect_network();
        puts("L'organisation que vous avez hack s'est rendue compte de la "
             "supercherie, et a jeté votre magnifique ordinateur...");
        free(computer);
      } else {
        puts("Vous n'avez actuellement pas d'ordinateur pret pour le "
             "hackingue, construisez en un d'abord");
      }
      break;

    case '3':
      message = malloc(sizeof(message->content));
      printf("Quelle note maléfique voulez vous laisser à votre cible : ");
      int len = read(0, message->content, sizeof(message->content));
      message->content[len - 1] = '\0';
      break;

    case '4':
      if (message) {
        write(1, message, sizeof(struct Computer));
      } else {
        puts("Vous n'avez pas encore écrit de message maléfique");
      }
      break;

    case '5':
      admin();
      break;

    case '0':
      exit(0);
      break;

    default:
      puts("Option non reconnue");
      break;
    }
  }

  return EXIT_SUCCESS;
}
