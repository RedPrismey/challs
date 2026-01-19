# asm

## Déploiement

### Serv

Rien

### Donner au challengers

- binaire

## Énoncé

## Writeup

### Reconnaissance

```text
❯ file chall
chall: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), statically linked, stripped
```

Dès le début on remarque plusieurs choses étrange. On ne voit pas d'artéfacts de gcc ou d'un autre compiler, et le binaire est statique et stripped. On va continuer un peu puis voir après ce qu'on peut en déduire.

`strings` nous donne aussi très peu de résultat. On peut quand même trouver des information intéressantes :

```text
Entrez votre nom d'utilisateur :
Entrez votre cl
 de licence : Bienvenue sur notre g
rateur de points, vous pouvez utiliser la licence comme flag pour ce challenge.
womp womp
```

Les accents sont toujours un peu funky avec strings, mais on comprends ce qui est dit (si jamais, vous pouvez ouvrir un hex editor pour vérifier ce qu'il y a, ici je considère qu'on peut deviner).

On a donc a priori un vérificateur de licence qui nous demande un nom d'utilisateur ainsi qu'une clé de licence.

### Mettre les mains dans le cambouis

UPDATE LES ADRESSES

Si on ouvre le binaire dans un logiciel de RE, on a pas de nom de fonction (le binaire est stripped, donc c'est normal). En regardant un peu, on se rend compte que la décompilation n'est pas très utile, et il n'y a pas d'appel à des fonctions de libc, que des syscalls et des appels à des fonctions interne. Avec cette information + le fait qu'il n'y ait pas d'artéfacts de compiler, on peut supposer que le challenge a été créé directement en assembleur (je peux le confirmer puisque c'est moi qui ai fait le challenge, mais cette information n'est pas nécessaire en soit pour solve).

Vu que comprendre le binaire en statique peut être un peu laborieux à cause du fait qu'il soit créé en asm, je vais proposer une solution en dynamique.

Par la suite je vais utiliser gdb (avec gef <https://github.com/hugsy/gef.git>, qui améliore un peu l'utilisation de gdb). Pareil, je vais faire appel à des notions (registres x86-64, syscalls linux, assembleur). Je ne vais pas faire de tuto en profondeur sur son utilisation, mais si vous voulez en savoir plus et dans tous les cas je vous met quelques ressources, n'hésitez pas à passer au club, je vous expliquerai.

On va run le binaire pour voir à quoi s'attendre. On voit que notre analyse du dessus par rapport à `strings` était plutôt bonne, parce qu'on nous demande un nom d'utilisateur, puis un clé de licence. Vu qu'a priori on a mis n'importe quoi, le programme va print `womp womp` et exit.

Vu  qu'on a pas de fonction comme `read` pour mettre un breakpoint et arriver dans la partie du code qui nous intéresse (je conseille assez peu de passer instruction par instruction dans la plupart des cas), on va utiliser la commande `catch syscall` de gdb. Vu qu'on sait qu'on print un prompt avant de read, on va naturellement placer un catch sur le syscall 1 (aka write, si vous voulez avoir le numéro des breakpoint, vous pouvez regarder une syscall table comme celle-ci <https://blog.rchapman.org/posts/Linux_System_Call_Table_for_x86_64/>). Comme un `catch` n'est pas un réel breakpoint, on va avoir l'exécution qui va être stoppée juste *après* avoir exécuté le write syscall.

Avec notre catch placé, on peut run et commencer à voir ce qu'il se passe. Dans la partie code de gef, on voit ça :

```asm
     0x40100a                  movabs rsi, 0x402000
     0x401014                  mov    edx, 0x21
     0x401019                  syscall
 →   0x40101b                  xor    rax, rax
     0x40101e                  xor    rdi, rdi
     0x401021                  movabs rsi, 0x4031e8
     0x40102b                  mov    edx, 0x32
     0x401030                  syscall
     0x401032                  dec    rax
```

On peut voir plus du code autour de nous avec `x/10i $rip-0x16` (pareil pour plus de détails sur comment utiliser gdb, n'hésitez pas à venir au club info), ce qui nous donne :

```asm
   0x401005: mov    edi,0x1
   0x40100a: movabs rsi,0x402000
   0x401014: mov    edx,0x21
   0x401019: syscall
=> 0x40101b: xor    rax,rax
   0x40101e: xor    rdi,rdi
   0x401021: movabs rsi,0x4031e8
   0x40102b: mov    edx,0x32
   0x401030: syscall
   0x401032: dec    rax
```

Vu qu'on connait l'ordre de passage des argument de l'ABI system V et les arguments du syscall write (sinon voir dans ressources, tout y est), dans `rdi`, on a le file descriptor dans lequel il faut écrire (ici 1, c'est à dire `stdout`), dans `rsi` on place l'adresse de ce qu'il faut écrire, et dans `rdx`, la quantité de caractères à écrire. Les 4 instructions avant notre position écrivent 0x21 (ou 33) caractères du buffer situé à l'adresse `0x402000` dans `stdout`.

On peut regarder ce qu'il y a à l'adresse `0x402000` avec `x/s 0x402000`, ce qui nous donne :

```text
"Entrez votre nom d'utilisateur : 
```

On retrouve ce qu'on a trouvé avec strings, et c'est bien ce qu'on voit affiché en premier (le message qui nous demande notre username), ça tombe bien et ça veut dire qu'on est au bon endroit.

Je vais accélérer le process de lecture des arguments, donc après, on a un read de maximum 50 char depuis `stdin` vers un buffer situé à l'adresse `0x4031e8`, donc d'après le message avant le read, `0x4031e8` est l'adresse de l'username. On va confirmer ça en avançant dans le programme. Je peux en effet rentrer qqch, et je choisis le magnifique username `aaaaaaaaaaaaaaaaaaaaaaaaaa`. `read` retourne la quantité de caractères lu, et le `dec rax` sert à ignorer le `\n` (que `read` compte).

Ensuite, `0x401035   mov QWORD PTR [rip+0x21e6], rax` enregistre la taille de l'username qu'on vient de prendre à l'adresse `0x403222` (mise en commentaire par gef).

On nous demande ensuite une clé de licence (je choisi `bbbbbbbbbbbbbbbbbbbbbbbbbb`), dont la taille sera stockée à `0x40325c`.

### Validation des inputs

Une fois qu'on a toutes les info dont on a besoin, on peut s'attendre à arriver sur la logique du programme, et en effet, on voit qu'on déplace notre nom d'utilisateur dans `rdi`, sa taille dans `rsi`, la licence dans `rdx` et sa taille dans `rcx`, suivi d'un appel de la fonction (située à `0x4010e7`).

On peut se douter que cette fonction va être celle qui vérifie les inputs, c'est à dire la fonction qu'on veut comprendre.

La première chose qui est faite, c'est de `mov` tous les arguments dans `r8` à `r11`, pour libérer la place pour les futurs call.

On prépare l'appel d'une fonction en mettant en 1e argument une valeur étrange dans `rdi` (à l'adresse `0x4020bb`. Si on regarde à quoi le buffer ressemble, on trouve ça :

```text
gef➤  x/16x 0x4020bb
0x4020bb: 0x00640041 0x0069006d 0x0069006e 0x00740073
0x4020cb: 0x00610072 0x006f0074 0x4e004972 0x41005300
0x4020db: 0x48005300 0xcc0d7b00 0xfac5fbf2 0xd0fef9bd
0x4020eb: 0xff00c4f8 0xffffffff 0xffffffff 0xffffffff
```

Je ne vais pas commenter le contenu plus que ça étant donné qu'on va voir ce qu'on peut en faire juste après.

On setup les autres arguments : 19 (valeur inconnue pour l'instant à priori) et un buffer vide à l'adresse `0x403264`, suite à quoi on appelle la fonction à `0x4011ff`.

#### La valeur mystérieuse

La fonction met à 0 `rax` ainsi que `rcx`. On a ensuite ces 2 instruction pouvant paraitre un peu mystérieuse :

```asm
0x401209                  mov    bl, BYTE PTR [rdi+rcx*1]
0x40120c                  mov    BYTE PTR [rdx+rax*1], bl
```

Ne paniquez pas, ce n'est pas si compliqué. Il suffit de se rappeler que `rdi` est l'adresse de notre buffer contenant la valeur étrange, `rcx = 0` (et que c'est le registre des compteurs de boucles) et que `rax = 0`. Dans ce cas, on va donc prendre l'octet `bizarre[0]` pour le placer dans `buffer_vide[0]`.

On incrémente ensuite `rax` de 1, et `rcx` de 2. On compare ensuite `rcx` à notre 2e argument, c'est à dire 19, si c'est `rcx < 19`, on recommence. Si vous avez du mal à comprendre, n'hésitez dessinez le flow de programme et regardez ce que font les 2-3 premières itérations.

Au fil des itérations, on commence à voir `Administrator` s'écrire dans notre buffer au départ vide. On a résolu notre mystère !

Cette fonction est donc une fonction de deobfuscation. L'obfuscation est une pratique qui consiste à camoufler des informations pour qu'elles soient plus difficile à comprendre/repérer. Mettre des 0 entre chaque caractère permet que l'username ne soit pas détecté par `strings` (étant donné que ça n'est en effet pas une chaine de caractères valide) et à complexifier la compréhension du programme (même si ici elle n'était pas trop méchante).

En réalité avec un peu d'instinct on aurait pu le voir depuis le dump hexadécimal, il y a un caractère valide, un 0, ainsi de suite, ou même on aurait pu ne pas s'embêter à comprendre et voir qu'on retourne `Administrator` à la fin et s'en satisfaire. Tout est une question d'habitude. Par précaution, il est toujours plus prudent de regarder tout ce qu'il se passe dans un programme (on ne sait pas, ça se trouve j'aurais pu mettre du code malveillant dans cette fonction en la faisant passer pour une innocente fonction de deobfuscation, même si c'est peu probable dans ce contexte de CTF).

#### Les checks

Juste après, on a un check :

```asm
0x401111                  cmp    r9, 0xd
0x401115                  jne    0x4011e6
```

Qui fail, étant donné que `r9 = 0x1a` (la taille de notre nom d'utilisateur) est différente de `0xd`. Si on prends le jump, on retourne 1, et dans `_start` on print `womp womp` avant d'exit. C'est donc le chemin qu'on ne veut pas prendre.

On pourrait ignorer le jump avec gdb (en modifiant manuellement la valeur de `r9` où avec un jump over), mais je choisis ici de modifier l'username que j'ai entré avec le bon username. A partir de maintenant, mon nom d'utilisateur est `Administrator`, et ma clé de licence reste `bbbbbbbbbbbbbbbbbbbbbbbbbb`.

Juste après, on appelle une fonction avec le le buffer contenant `Administrator`, notre username et 13 comme arguments. Cette fonction boucle en comparant caractère par caractère les deux chaines de caractères. Étant donné que j'ai modifié l'username, les tests vont passer et la fonction va retourner 0.

Le prochain check est :

```asm
0x40113a                  cmp    r11, 0x1c
0x40113e                  jne    0x4011e6
```

Ce check compare la longueur de notre licence avec 28.

Actuellement, on a pas la bonne longueur, mais contrairement à précédemment, je ne sais pas déjà ce que je dois mettre pour valider, je vais donc juste modifier la valeur de `r11` pour forcer le test à passer avec `set $r11 = 0x1c`, et espérer que je trouve plus d'informations après.

Juste après, on appelle la même fonction que précédemment (`0x4011ff`) avec un buffer similaire à celui contenant `Administrator`, 13 et un buffer vide. Étant donné qu'on a déjà vu que cette fonction deobfuscate le buffer en entrée, on peut juste passer à la fin pour en voir le résultat, qui est `INSASH{`. C'est un super bon signe étant donné que ça correspond au format de flag.

Juste après, on appelle de nouveau la fonction de comparaison (`0x401218`) entre notre string deobfuscated et les 7 premiers caractères de notre licence.

Bien évidemment, ça ne fonctionne pas, mais pareil qu'avant, on `set $rax = 0` pour bypass le check.

On recompare le caractère à l'index 27 de la clé de licence et `}` (`0x7d`) (cette fois si sans la fonction, un caractère étant juste un octet, on peut utiliser l'instruction `cmp` directement).

La licence que j'ai choisi au début ne fait pas 27 caractères, mais c'est pas grave, on va juste avoir une lecture en dehors des octets initialisés (sachant qu'on a 50 octets de réservé pour ce buffer), ce qui nous donnera 0. Encore une fois, on bypass.

#### Un peu de nouveauté

A partir de ce point, j'ai modifié ma licence en `INSASH{bbbbbbbbbbbbbbbbbbbb}`. Je me suis basée sur la comparaison de taille et sur les parties du flag connues, avec toujours des "b" en placeholder.

Une fois tout ces tests passé, on arrive à quelque chose qu'on avait pas encore vu : un appel de fonction (`0x401239`) avec comme arguments :

- 20 (28 - 8, 28 étant la taille de notre clé de licence et 8 étant la taille de ce qu'on connait, aka `INSASH{}`)
- pointeur vers notre licence + 7
- l'adresse du buffer contenant `INSASH{`

On va donc à priori agir sur l'intérieur de la licence, dans notre cas `bbbbbbbbbbbbbbbbbbbb`.

Dans un premier temps, on clear `rcx`, `r9` et `r10`, puis on copie la taille de notre string dans `rbx`, avant de faire une comparaison entre `cl` et 6. Ici, ça n'est pas égal, donc on peut ignorer cette partie pour la 1e itération. Dans `r8`, on prend la taille totale de la chaine de caractères, à qui on soustrait `rbx`, ce qui nous donne 0 pour l'instant (on peut se douter que `rbx` va diminuer, et donc que `r8` va être l'indice de notre caractère).

On vérifie ensuite si cet indice +1 est toujours dans notre chaine de caractère. On compare ensuite notre caractère à `0x3d` ("="), si c'est égal, on skip à la fin. Ensuite on prend la valeur à l'indice égal à notre caractère (l'équivalent de `buffer[string[i]]`). On ne sait pas ce que c'est, mais on peut regarder :

```text
0x4020ee: 0xffffffff 0xffffffff 0xffffffff 0xffffffff
0x4020fe: 0xffffffff 0xffffffff 0xffffffff 0xffffffff
0x40210e: 0xffffffff 0xffffffff 0x3effffff 0x3fffffff
0x40211e: 0x37363534 0x3b3a3938 0xffff3d3c 0xffff40ff
0x40212e: 0x020100ff 0x06050403 0x0a090807 0x0e0d0c0b
0x40213e: 0x1211100f 0x16151413 0xff191817 0xffffffff
0x40214e: 0x1c1b1aff 0x201f1e1d 0x24232221 0x28272625
0x40215e: 0x2c2b2a29 0x302f2e2d 0xff333231 0xffffffff
0x40216e: 0xffffffff 0xffffffff 0xffffffff 0xffffffff
0x40217e: 0xffffffff 0xffffffff 0xffffffff 0xffffffff
0x40218e: 0xffffffff 0xffffffff 0xffffffff 0xffffffff
0x40219e: 0xffffffff 0xffffffff 0xffffffff 0xffffffff
0x4021ae: 0xffffffff 0xffffffff 0xffffffff 0xffffffff
0x4021be: 0xffffffff 0xffffffff 0xffffffff 0xffffffff
0x4021ce: 0xffffffff 0xffffffff 0xffffffff 0xffffffff
0x4021de: 0xffffffff 0xffffffff 0xffffffff 0xffffffff
```

On voit que la grande majorité des cas donnent `0xff`, mais certains caractères nous donnent autre chose, par exemple, "A" nous donne 0, "B", 1, etc. Avec un petit script, on trouve que l'alphabet est `ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=`. On remarque qu'on a l'air de traiter avec de la base 64 (par rapport à l'alphabet et au "="), on peut donc supposer que la fonction sert à décoder un input base 64, de taille 20 qu'on met dans notre buffer. Si on regarde rapidement la suite de la fonction, on remarque de l'arithmétique assez obscure, mais étant donné qu'on a déjà une idée de ce que la fonction fait en fonction du contexte, on peut juste vérifier la sortie et tester notre théorie (par chance, notre input est de la base 64 valide).

En sortie, on trouve `0xb66ddbb66ddbb66d`, ce qui est le bon résultat (si vous le faites sur un tool en ligne, les caractères invalides seront pas toujours affichés, mais vous verrez les "m", et si vous être toujours pas convaincu•es, vous pouvez tester avec autre chose).

### La fin

On compare la valeur de retour de notre fonction de décodage de base64, mais on a pas la bonne valeur à 2 près. On bypass ce check, étant donné qu'il n'est pas super important (mais c'est la taille de l'output qui a été écrit dans notre buffer).

Une fois que c'est fini, on appelle ensuite une nouvelle fonction, avec un buffer notre username admin et 13, voyons voir ce qu'elle fait. On a une boucle simple où on ajoute le contenu d'un buffer mystère (situé à `0x4020e0`) à notre username admin. On peut juste regarder le résultat pour voir quel est le résultat, et on trouve `N0_d3c0mp1l3r` (il faut couper à partir de 13, parce que la chaine de caractères n'est pas null terminated, donc gdb ne sait pas où s'arrêter).

Et enfin, on appelle la fonction de comparaison qu'on a déjà vu de nombreuses fois maintenant entre notre output décodé en base64 et `N0_d3c0mp1l3r`. Bien évidemment, le check fail, mais maintenant, on sait absolument tout ce qu'il y a à mettre comme input pour valider : `Administrator` en username et `INSASH{TjBfZDNjMG1wMWwzcg==}` en clé de licence (`TjBfZDNjMG1wMWwzcg==` étant la version encodée en base64 de `N0_d3c0mp1l3r`).

Et voilà, on a enfin fini

### Solution

Username: Administrator
flag : `INSASH{TjBfZDNjMG1wMWwzcg==}`

### Ressources

- <https://insash.org/article/ateliers/les-bases-du-langage-assembleur>
- <https://wiki.osdev.org/CPU_Registers_x86-64>
- <https://blog.rchapman.org/posts/Linux_System_Call_Table_for_x86_64/>
- <https://cs.brown.edu/courses/cs033/docs/guides/x64_cheatsheet.pdf>
- <https://web.stanford.edu/class/cs107/resources/x86-64-reference.pdf>
- <https://www.cs.uaf.edu/2017/fall/cs301/reference/x86_64.html>
- <https://wiki.osdev.org/System_V_ABI#x86-64>
