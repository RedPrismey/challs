global _start

SECTION .rodata
  usr_prompt              dq    "test wola"

SECTION .text
_start:
  mov rdi, usr_prompt

  mov rax, 60
  xor rdi, rdi
  syscall
