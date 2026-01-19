# Pre-analyse

- d'après le nom, on peut en déduire que c'est du rust
- elf x64 stripped (`file rusty_rev` : `rusty_rev: ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, BuildID[sha1]=38e2762459179204b5080843a620fed52c0abe45, for GNU/Linux 4.4.0, stripped`)
- `strings` ne nous donne pas grand chose, on peut noter (en cherchant pas mal, pas obligatoire) `Please input your password :` (input line for password), `gdb error, please reboot the computer` (on peut se douter que l'erreur gdb n'est pas supposée être dans le binaire), `ggwp, you can validate using this password` (message qui peut indiquer l'endroit d'arrivée), `Wrong password, reporting incident to the admin` (message de fail, aussi probablement près de la fin) et `ST0P17` et `5734M3D` (il faut vrmt chercher pour les trouver, et on peut noter que les strings sont sus, mais ce n'est pas le flag)
- `rz-bin -z` (pour avoir la section string du binaire) on peut trouver :
░░░░░░░░░░░░░░░░░█████░░░░░░░░░░░░░░░░░░
░░░░░░░░░░░░░████░░░░░███░░░░░░░░░░░░░░░
░░░░██████████░░░░░░████████████░░░░░░░░
░░░░█░░░░░░░█░░░░████░░░░░░░░░░██░░░░░░░
░░░██░░░░░░██░░░░░█░░░░░░░░░░░██░░░░░░░░
░░░█░░░░░░░█░░░░░░███░░░░░█████░░░░░░░░░
░░░░█░░░░░░█░░░░░░░░░███████░░░░░░░░░░░░
░░░░░███████░░░░░░░░░░░░░░░█░░░░░░░░░░░░
░░░░░░░░░░░█░░░███████░░░░░█░░░░░░░░░░░░
░░░░░░░░░░░█░░░█░░░░░█░░░░░█░░░░░░░░░░░░
░░░░░░░░░░░█░░░█░░░░░█░░░░█░░░░░░░░░░░░░
░░░░░░░░░░░█░░░█░░░░░█░░░░█░░░░░░░░░░░░░
░░░░░░░░░░░█░░░█░░░░░█░░░░█░░░░░░░░░░░░░
░░░░░░░░░░░█░░░█░░░░░██████░░░░░░░░░░░░░
░░░░░░░░░░░█████░░░░░░░░░░░░░░░░░░░░░░░░
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
c'est marrant, mais ça ne sert globalement à rien

# Analyse statique

-
