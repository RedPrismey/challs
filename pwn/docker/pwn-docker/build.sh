#!/bin/sh

cd data # il y a surement un moyen moins ghetto de le faire mais ¯\_()_/¯

echo "[*] Building the challenge..."
gcc ../src/*.c -o chall -fno-stack-protector -Wstringop-overflow=0
echo "[+] Done building!"

echo "[*] Building the challenge..."
cp chall ../unpatched_chall
echo "[+] Done building!"

echo "[*] Linking to libc..."
patchelf --set-interpreter ld-linux-x86-64.so.2 chall
patchelf --set-rpath . chall
echo "[+] Done linking!"

cd ..
