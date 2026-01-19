# Intro

## Déploiement

### Serv

- `docker compose up` (gérer le port et avoir build l'image nsjail avant)

### Donner au challenger

- Port + IP du docker
- unpatched binaire
- code source

## Énoncé

Nous avons retrouvé une application laissée par ./evil.sh ! On a déjà réussi à établir un contact avec eux, donc l'utiliser normalement ne nous est pas utile, mais j'ai entendu dire que vous vous y connaissez en exploitation de binaire. Essayer d'accéder au serveur qui héberge cette application pour pouvoir y trouver plus d'informations sur cette organisation.

## Writeup

Pour ce writeup, je vais beaucoup détailler les concepts de base pour ne plus avoir à les détailler dans d'autres writeup. Si jamais vous avez des question ou des suggestions, n'hésitez pas à me contacter sur discord (`.prismey`) ou à passer au club.

### Reconnaissance

#### Les classico

`❯ file chall
chall: ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, BuildID[sha1]=090b8ef39f85f9858bdb824df81b0b095dd4c88f, for GNU/Linux 4.4.0, not stripped`

`❯ checksec --file=chall
RELRO           STACK CANARY      NX            PIE             RPATH      RUNPATH      Symbols     FORTIFY  Fortified  Fortifiable  FILE
Partial RELRO   No canary found   NX enabled    PIE enabled     No RPATH   No RUNPATH   32 Symbols  No       0          2            chall`

On voit donc que c'est un binaire x86-64 unstripped, avec toutes les protections classiques sauf le canary.

En utilisant `strings` dessus, on voit aussi la bannière, le prompt pour le message et enfin quelque chose d'intéressant : `/bin/bash`. On verra pourquoi c'est là plus tard.

#### Le code

En regardant le code, on voit qu'on a une fonction `boot` qui ne donne pas de fonctionnalités (désactive le buffering de stdin, stdout et stderr pour les curieux•ses), et toutes les fonctionnalités se trouvent dans `main`.

On a deux variables, `char input[50]` et `int debug_mode = 0`. On peut se douter que `input` va accueillir notre message, mais `debug_mode` cache probablement quelque chose d'intéressant, et en effet, quelques lignes plus loin, on voit ces lignes :

```C
  if (debug_mode) {
    system("/bin/bash");
  }
```

Notre objectif va donc être de changer la valeur de `debug_mode` pour que le test passe, et donc qu'on pop un shell.

On voit aussi une ligne intéressante : `read(0, input, 0x50);`. Si vous vous souvenez, la taille de input est 50, pas 0x50 (80), ce qui va nous permettre de faire des choses intéressantes.

Quand on run le programme, on voit une magnifique bannière, ainsi qu'un prompt qui nous demande d'envoyer un message, mais rien de surprenant par rapport à la lecture du code.

### Exploitation

Le challenge n'est pas très difficile, et il est possible de flag juste avec un petit peu de fuzzing (si vous mettez un message suffisamment long notamment), mais on va essayer d'avoir une approche un peu plus intentionnelle.

Comme dit précédemment, notre objectif va être de modifier la valeur de la variable `debug_mode` pour que le test `if (debug_mode)` passe. En C, ce test est équivalent à `if (debug_mode != 0)`, donc mettre n'importe quelle valeur autre que 0 dans `debug_mode` va faire réussir le test.

### Explications des concepts

Pour faire ça, on va utiliser un **buffer overflow**. En C, un buffer (ou tampon) est une zone de mémoire qu'on réserve pour y mettre des choses. Par exemple, quand vous écrivez `char string[10]`, vous réservez implicitement un buffer qui permet de stocker 12 `char`. Un buffer overflow est simplement l'action d'écrire au delà de la mémoire allouée, par exemple écrire un 11e `char` dans notre variable `string` (qui ne peut en contenir que 10). Qu'est-ce qu'il se passe si on écrit là où on est pas supposé écrire ? Ca dépend, et je ne vais expliciter que le cas qui concerne notre challenge, mais si vous voulez creuser (pour résoudre les challenges suivant ou pour votre curiosité), je vous met des ressources en dessous.

Dans le cadre de notre challenge, ce qu'il y a après `input`, c'est `debug_mode`. Comment je le sais ? j'ai créé le challenge :p. Plus sérieusement, ce n'est pas parce que `input` est définit avant `debug_mode` que les deux variables sont dans cet ordre dans la mémoire. En réalité, si vous utilisez `gcc` sans l'option `-fno-stack-protector` pour compiler le programme, elles seront dans l'ordre inverse (d'ailleurs si quelqu'un sait comment controller plus finement l'ordre des variables avec `gcc`, j'aimerai bien savoir). Pour vérifier l'ordre dans lequel elles sont, il y a plusieurs manières, mais je vais utiliser `cutter`, frontend de `rizin`, un logiciel de reverse engineering.

Quand j'ouvre mon programme avec rizin et que je regarde la fonction main, je vois ceci au tout début :

```text
; var void *buf @ stack - 0x40
; var unsigned long long var_ch @ stack - 0x4
```

Comme le programme est compilé, on ne retrouve pas les noms de variable, mais on peut déduire que `buf` correspond à `input` et `var_ch` à `debug_mode` (les variables au dessus de celles que j'ai montrées sont des copies de `argc` et `argv`). On voit `@ stack - 0x48`. Je ne vais pas expliquer en détail ce qu'est le stack, pour l'explication il suffit de considérer que c'est l'endroit où les variables sont stockées (à la suite) (encore une fois il y a des ressources à la fin). Comme `0x48` (72) est inférieur à `0xc` (12), quand on va écrire au delà des limites de `string`, on va donc écrire sur `debug_mode`. Ca tombe bien, c'est la variable qu'on doit modifier (quel bel hasard, on dirait presque que c'est fait exprès).

### Enfin on casse de trucs

Maintenant qu'on a vu tout ça (ou si vous avez skip tout ce que j'ai écrit avant ••), écrivons l'exploit. On va dans un premier temps remplir `input` avec par exemple des 'A', l'espace entre les deux avec des 'B', puis `debug_mode` avec un 'C' (n'importe quelle autre lettre marcherait à par \x00, l'important c'est juste que `debug_mode` soit différent de 0).

Donc notre payload va être :

```python
payload = b"A" * 50 + b"B" * 10 + b"C"
```

(ma solution complète va être mise à côté, vu que ça serait juste long pour rien ici).

On envoie ça, et pouf on a notre shell ! Félicitation, vous pouvez juste `cat flag.txt` qui contient `INSASH{34sy_bU773r_0v3r710w}` et pouf, le chall est validé, bravo.

## Ressources

Si vous voulez plus d'info, vous pouvez regarder ici : (ou venir au club, je vous expliquerai avec plaisir :])

- <https://en.wikipedia.org/wiki/Buffer_overflow>
- <https://owasp.org/www-community/attacks/Buffer_overflow_attack>
- <https://ctf101.org/binary-exploitation/what-is-the-stack/>
- <https://book.hacktricks.wiki/en/binary-exploitation/stack-overflow/index.html>
- <https://en.wikipedia.org/wiki/Call_stack>

N'hésitez pas à dessiner le stack, ça aide beaucoup à comprendre comment il fonctionne je trouve.
