#!/bin/python

from sys import argv, exit
from pwn import context, gdb, process, remote


def pwn(p: remote | process):
    # TODO: pwn

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
