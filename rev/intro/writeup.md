# intro (reverse)

## Déploiement

### Serv

Rien

### Donner au challenger

- binaire

## Énoncé

Nous avons trouvé une console d'administration de ./evil.sh ! Malheureusement nous n'avons pas le mot de passe pour y accéder. J'ai entendu dire que vous aviez des compétence en reverse engineering, essayez de retrouver le mot de passe pour entrer dans cette console.

Le format du flag est INSASH{mot_de_passe_trouvé}.

## Writeup

### Reconnaissance

```
❯ file intro
intro: ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, BuildID[sha1]=4995724b2d24ab8746a87faf9990413175dee33a, for GNU/Linux 4.4.0, not stripped
```

On a un executable x86-64 linux pas stripped, plutôt classique.

`string` nous donne pas mal d'informations (qqch qui semble être affiché pour nous accueillir, qqch pour quand on gagne et "nuh uh" pour quand on se loupe), mais pas de flag en vue.

### le dur du sujet

Ici, je vais donner une solution en statique, si jamais vous voulez essayer de le faire en dynamique ou tout simplement que vous avez une question, vous pouvez me contacter sur discord : `.prismey`.

Quand on ouvre le fichier dans notre outil de reverse préféré (cutter dans mon cas), et qu'on regarde les fonctions, on voit assez vite une fonction qui s'appelle `check_password`. Probablement une bonne piste, mais regardons d'abord ce que `main` fait.

#### main

En effet, on appelle dans un premier temps la méthode boot, puis on fait des opération sur des strings (qu'on a pas besoin de comprendre ici, c'est juste l'allocation de la mémoire pour une string et la lecture depuis stdin), puis on appelle `check_password` et enfin on utilise la valeur de retour pour print soit "nuh uh", soit le texte de victoire.

↓ décompilation de rizin (le backend de cutter) pour illustrer mes propos, une autre décompilation ou même regarder l'assembleur permet de trouver les mêmes informations, juste dans un format différent, à vous de trouver le format qui vous convient le mieux.

```c
undefined8 main(int argc, char **argv)
{
    char cVar1;
    undefined8 uVar2;
    int64_t in_FS_OFFSET;
    char **var_78h;
    int var_6ch;
    int64_t var_68h;
    int64_t var_48h;
    int64_t canary;
    int64_t var_10h;
    
    canary = *(int64_t *)(in_FS_OFFSET + 0x28);
    method.std::__cxx11::basic_string_char__std::char_traits_char___std::allocator_char___.basic_string
              ((int64_t)&var_68h);
    boot()();
    method.std::basic_istream_char__std::char_traits_char_____std.operator___char__std::char_traits_char___std::allocator_char____std::basic_istream_char__std::char_traits_char______std::__cxx11::basic_string_char__std::char_traits_char___std::allocator_char
              (std::cin, &var_68h);
    method.std::__cxx11::basic_string_char__std::char_traits_char___std::allocator_char___.basic_string_std::__cxx11::basic_string_char__std::char_traits_char___std::allocator_char____const
              ((int64_t)&var_48h, (int64_t)&var_68h);
    cVar1 = check_password(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >)
                      ((int64_t)&var_48h);
    method.std::__cxx11::basic_string_char__std::char_traits_char___std::allocator_char___._basic_string
              ((int64_t)&var_48h);
    if (cVar1 == '\0') {
        uVar2 = method.std::basic_ostream_char__std::char_traits_char_____std.operator____std::char_traits_char____std::basic_ostream_char__std::char_traits_char______char_const
                          (std::cout, "nuh uh");
        method.std::ostream.operator___std::ostream______std::ostream
                  (uVar2, 
                   _std::basic_ostream<char, std::char_traits<char> >& std::endl<char, std::char_traits<char> >(std::basic_ostream<char, std::char_traits<char> >&)
                  );
    } else {
        uVar2 = method.std::basic_ostream_char__std::char_traits_char_____std.operator____std::char_traits_char____std::basic_ostream_char__std::char_traits_char______char_const
                          (std::cout, 
                           "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  \n                Welcome back                      \n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  \n        ./evil.sh console version 3.2             \n           Administrator interface                \n# ls                                              \n  intro.cpp  intro  flag.txt                       \n# cat flag.txt                                    \n  Bravo, vous pouvez valider ce challenge avec le flag INSASH{password} (en remplaçant password par le mot de passe que vous avez trouvé)\n# exit                                            \n"
                          );
        method.std::ostream.operator___std::ostream______std::ostream
                  (uVar2, 
                   _std::basic_ostream<char, std::char_traits<char> >& std::endl<char, std::char_traits<char> >(std::basic_ostream<char, std::char_traits<char> >&)
                  );
    }
    method.std::__cxx11::basic_string_char__std::char_traits_char___std::allocator_char___._basic_string
              ((int64_t)&var_68h);
    if (canary != *(int64_t *)(in_FS_OFFSET + 0x28)) {
    // WARNING: Subroutine does not return
        __stack_chk_fail();
    }
    return 0;
}

```

#### check_password

Maintenant qu'on a vu ça, on va s'attaquer à `check_password`. En regardant bien, on peut voir deux éléments important. Déjà, une boucle, dont on parlera un peu plus tard et ensuite, une chaine de caractère étrange (`6w\tw)\x06w6\x1b\x1ct6\x1bu7\x1b*ts\x1bw*\a\x16=4sut\n`) assignée à `var_48h` (nom de variable généré automatiquement par rizin).

Regardons maintenant ce qu'il se passe dans la boucle. On se rend compte qu'il s'agit d'une boucle for, puisqu'on a une variable qui s'incrémente de 1 à chaque itération et on break quand cette variable est égale, et la seule chose qui est faite dans cette boucle c'est ça `*(uint8_t *)var_a0h = *(uint8_t *)var_a0h ^ 0x44;` (tout le reste étant là pour le control flow). L'opération `^` correspond à un XOR, qui est souvent utilisé pour camoufler ou "chiffrer" des informations (un XOR tout seul n'est pas un chiffrement, ou en tout cas pas un chiffrement sur seul puisqu'il est trivial à inverser).

Il faut maintenant savoir avec quoi xor. On a vu précédemment que `var_48h` contient une string suspecte, et justement, juste en dessous, on voit `method.bool_std.operator___char__std::char_traits_char___std::allocator_char____std::__cxx11::basic_string_char__std::char_traits_char___std::allocator_char____const___std::__cxx11::basic_string_char__std::char_traits_char___std::allocator_char____const` (autrement dit `==`) entre cette variable et `var_68h`, dans laquelle est copié l'argument de la fonction, autrement dit le mot de passe qu'on vient de rentrer.

Essayons de XOR notre string bizarre dans `var_48h` et 0x44 :

```python
expected = "6w\tw)\x06w6\x1b\x1ct6\x1bu7\x1b*ts\x1bw*\a\x16=4sut\n" 

out = ""

for i in expected:
    out += chr(ord(i) ^ 0x44)

print(out)
```

Et magie, on trouve `r3M3mB3r_X0r_1s_n07_3nCRyp710N`. Si on lance le programme et qu'on essaye ce mot de passe, le texte de victoire s'affiche. C'est donc flag, et il suffit d'ajouter le INSASH{} pour valider sur le serveur.
