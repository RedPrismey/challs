#include <iostream>

char BANNER[] =
    "=================================================================\n"
    "  _________      /\\/\\              .__.__       .__\n"
    "  \\_   ___ \\  /\\ \\ \\ \\   _______  _|__|  |      |  |__   ______\n"
    "  /    \\  \\/  \\/  \\ \\ \\_/ __ \\  \\/ /  |  |      |  |  \\ /  ___/\n"
    "  \\     \\____ /\\   \\ \\ \\  ___/\\   /|  |  |__    |   Y  \\\\___ \\\n"
    "   \\______  / \\/    \\ \\ \\___  >\\_/ |__|____/ /\\ |___|  /____  >\n"
    "          \\/         \\/\\/   \\/               \\/      \\/     \\/\n"
    "                        evil console v0.0.3\n"
    "=================================================================\n\n"
    "login: admin                              \n"
    "password: ";

char WIN[] =
    "\n"
    "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  \n"
    "                Welcome back                      \n"
    "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  \n"
    "       C:\\\\evil.sh console version 3.2          \n"
    "           Administrator interface                \n"
    "# ls                                              \n"
    "  intro.cpp  intro  flag.txt                      \n"
    "# cat flag.txt                                    \n"
    "  Bravo, vous pouvez valider ce challenge avec le flag INSASH{password} "
    "(en remplaçant password par le mot de passe que vous avez trouvé)\n"
    "# exit                                            \n";

void boot() { std::cout << BANNER; }

bool check_password(std::string pass) {
  std::string crypted = pass;
  std::string expected =
      "\x36\x77\x9\x77\x29\x6\x77\x36\x1b\x1c\x74\x36\x1b\x75\x37\x1b\x2a\x74"
      "\x73\x1b\x77\x2a\x7\x16\x3d\x34\x73\x75\x74\xa";

  for (char &c : crypted) {
    c ^= 0x44;
  }

  return (expected == crypted);
}

int main(int argc, char *argv[]) {
  std::string password;

  boot();

  std::cin >> password;

  if (check_password(password)) {
    std::cout << WIN << std::endl;
  } else {
    std::cout << "nuh uh" << std::endl;
  }

  return 0;
}
