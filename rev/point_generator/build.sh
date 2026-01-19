#!/bin/bash
#gcc main.o -o main -Wl,-z -Wl,noseparate-code --nostdlib -static
# https://stackoverflow.com/questions/65037919/minimal-executable-size-now-10x-larger-after-linking-than-2-years-ago-for-tiny#65038281

nasm -f elf64 $1.nasm -o $1.o
ld $1.o -o $1 -s
rm $1.o
