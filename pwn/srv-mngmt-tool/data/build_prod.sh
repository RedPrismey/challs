#!/bin/sh

echo "[*] Building the challenge..."
gcc ../src/main.c -o chall -fPIE -pie -fno-stack-protector
echo "[+] Done building!"

echo "[*] Linking to libc..."
patchelf --set-interpreter ld-linux-x86-64.so.2 chall
patchelf --set-rpath . chall
echo "[+] Done linking!"
