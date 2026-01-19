global _start

DEFAULT REL

SECTION .rodata
  usr_prompt              dq    "Entrez votre nom d'utilisateur : "
  usr_prompt_len          equ   $ - usr_prompt

  licence_prompt          db    "Entrez votre clé de licence : "
  licence_prompt_len      equ   $ - licence_prompt

  win_text                db    "Bienvenue sur notre générateur de points, vous pouvez utiliser la licence comme flag pour ce challenge.", 0xa
  win_text_len            equ   $ - win_text

  lose_text               db    "womp womp", 0xa
  lose_text_len           equ   $ - lose_text

  obf_admin_username      db    "A", 0, "d", 0, "m", 0, "i", 0, "n", 0, "i", 0, "s", 0, "t", 0, "r", 0, "a", 0, "t", 0, "o", 0, "r" ; for obfuscation
  obf_admin_username_len  equ   $ - obf_admin_username
  admin_username_len      equ   13

  total_licence_len       equ   28

  obf_licence_start       db    "I", 0, "N", 0, "S", 0, "A", 0, "S", 0, "H", 0, "{"
  obf_licence_start_len   equ   $ - obf_licence_start

  max_input_len           equ   50

  cipher_table            db    13, -52, -14, -5, -59, -6, -67, -7, -2, -48, -8, -60, 0
  ; ↑ table containing the number needed to transform the "Administrator" string into the flag

  b64_table:              db    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x3e,0xff,0xff,0xff,0x3f,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0xff,0xff,0xff,0x40,0xff,0xff,0xff,0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0xff,0xff,0xff,0xff,0xff,0xff,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff
  ; ↑ table where b64_table[char_ascii_value] = char_b64_value, and with 0xff for invalid values
; -------------------------------


SECTION .bss
    username        resb   max_input_len
    username_len    resb   8         ; 8 bytes for the actual length of username

    licence         resb   max_input_len
    licence_len     resb   8

    admin_username  resb   admin_username_len

    output          resb   max_input_len ; buffer operations
; ------------------------------


SECTION .text
_start:
  ; ---[ syscall(write(STDOUT, usr_prompt, usr_prompt_len)) ]--- ;
  mov rax, 1
  mov rdi, 1
  mov rsi, usr_prompt
  mov rdx, usr_prompt_len
  syscall

  ; ---[ syscall(read(STDIN, username, max_input_len)) ]--- ;
  xor rax, rax
  xor rdi, rdi
  mov rsi, username
  mov rdx, max_input_len
  syscall

  dec rax  ; to ignore the \n
  mov [username_len], rax  ; log the size of username

  ; ---[ syscall(write(STDOUT, licence_prompt, licence_prompt_len)) ]--- ;
  mov rax, 1
  mov rdi, 1
  mov rsi, licence_prompt
  mov rdx, licence_prompt_len
  syscall

  ; ---[ syscall(read(STDIN, licence, max_input_len)) ]--- ;
  xor rax, rax
  xor rdi, rdi
  mov rsi, licence
  mov rdx, max_input_len
  syscall

  dec rax  ; to ignore the \n
  mov [licence_len], rax  ; log the size of licence

  ; ---[ check(username, username_len, licence, licence_len) ]--- ;
  mov rdi, username
  mov rsi, [username_len]
  mov rdx, licence
  mov rcx, [licence_len]
  call check

  cmp rax, 0
  jne .fail

  ; ---[ syscall(write(STDOUT, win_text, win_text_len)) ]--- ;
  mov rax, 1
  mov rdi, 1
  mov rsi, win_text
  mov rdx, win_text_len
  syscall

  jmp .exit

  .fail:
    ; ---[ syscall(write(STDOUT, lose_text, lose_text_len)) ]--- ;
    mov rax, 1
    mov rdi, 1
    mov rsi, lose_text
    mov rdx, lose_text_len
    syscall

  .exit:
    ; ---[ syscall(exit(0)) ]--- ;
    mov rax, 60
    xor rdi, rdi
    syscall
; ------------------------------

; ===( check
;   Arguments:
;     rdi: pointer to the buffer containing the username
;     rsi: length of the username string
;     rdx: pointer to the buffer containing the licence
;     rcx: length of the licence string
;   Return value:
;     rax: 0 if the username and licence is correct, 1 otherwise
; )===
check:
  ; Internal variables:
  ;   r8:  pointer to the buffer containing the username
  ;   r9:  length of the username string
  ;   r10: pointer to the buffer containing the licence
  ;   r11: length of the licence string

  mov r8,  rdi ; to save the username pointer
  mov r9,  rsi ; to save username_len
  mov r10, rdx ; to save the licence pointer
  mov r11, rcx ; to save licence_len

  ; ---[ deobfuscate_string(obf_admin_username, obf_admin_username_len, admin_username) ]--- ;
  mov rdi, obf_admin_username
  mov rsi, obf_admin_username_len
  mov rdx, admin_username
  call deobfuscate_string

  cmp r9, admin_username_len  ; check if the username input is the right size
  jne .fail

  ; ---[ compare_strings(admin_username, username, length) ]--- ;
  mov rdi, admin_username
  mov rsi, r8
  mov rdx, r9
  call compare_strings

  cmp rax, 0 ; if (username != admin_username)
  jne .fail

  cmp r11, total_licence_len  ; if (input_licence_len != total_licence_len)
  jne .fail

  ; ---[ deobfuscate_string(obf_licence_start, obf_licence_start_len, output) ]--- ;
  mov rdi, obf_licence_start
  mov rsi, obf_licence_start_len
  mov rdx, output
  call deobfuscate_string

  ; ---[ compare_strings(licence_start, licence[0:7], length) ]--- ;
  mov rdi, output
  mov rsi, r10
  mov rdx, rax
  call compare_strings

  cmp rax, 0 ; if the beginning of the licence is not "INSASH{"
  jne .fail

  mov al, [r10 + total_licence_len - 1] ; mov last char of licence in al
  cmp al, "}"  ; check if the last char is "}"
  jne .fail

  ; ---[ b64_decode(20, licence[7:27], output) ]--- ;
  ; to decode the b64 part of the licence
  mov rdi, 20
  add r10, 7
  mov rsi, r10
  sub r10, 7
  mov rdx, output
  call b64_decode

  cmp rax, 13
  jne .fail

  ; ---[ crypt(admin_username, admin_username_len) ]--- ;
  mov rdi, admin_username
  mov rsi, admin_username_len
  call crypt

  ; ---[ compare_strings(crypted_admin_username, b64_decoded_licence, length) ]--- ;
  mov rdi, admin_username
  mov rsi, output
  mov rdx, admin_username_len
  call compare_strings

  cmp rax, 0 ; if b64_decoded_licence != crypted_admin_username
  jne .fail

  xor rax, rax
  ret

  .fail:
    mov rax, 1
    ret
; ------------------------------


; ===( crypt
;   Arguments:
;     rdi: pointer to the buffer that contain the data to encrypt (will be overwritten)
;     rsi: length of the string in the buffer
; )===
crypt:
  mov rcx, rsi

  ; ---[ for (i = length; i > 0; i--) ]--- ;
  .loop:
    mov al, [cipher_table + rcx - 1] ; table containing values to add to the chars to obtain the encrypted output
    add [rdi + rcx - 1], al          ; add values to "Administrator" to encrypt

    dec rcx
    jnz .loop

  ret
; ------------------------------

; ===( deobfuscate_string
;   Arguments:
;     rdi: pointer to the buffer containing the obfuscated string
;     rsi: length of the obfuscated string
;     rdx: pointer to the output buffer
;   Return value:
;     rax: length of the reconstructed string
; )===
deobfuscate_string:
  xor rcx, rcx ; index for the obfuscated string
  xor rax, rax ; index for the output
  .loop:
    mov bl, [rdi + rcx]
    mov [rdx + rax], bl

    inc rax
    add rcx, 2
    cmp rcx, rsi
    jl .loop

  ret
; ------------------------------


; ===( compare_strings
;   Arguments:
;     rdi: pointer to the buffer containing the first string
;     rsi: pointer to the buffer containing the second string
;     rdx: length of the strings (it is assumed both are the same size)
;   Return value:
;     rax: 0 if they are the same, 1 otherwise
; )===
compare_strings:
  ; ---[ for (i = 0; i < length; i++) ]--- ;
  xor rcx, rcx
  .loop:
    mov al, [rdi + rcx] ; char of the first string
    cmp al, [rsi + rcx] ; if string1[i] != string2[i]
    jne .different      ; they aren't the same

    inc rcx
    cmp rcx, rdx
    jl .loop

  xor rax, rax ; if we ended up here, then the strings were the same
  ret

  .different:
    mov rax, 1 ; if we ended up here, then the strings were different
    ret
; -------------------------------


; ===( b64_decode
;   Arguments:
;     rdi: size of the input in the input buffer
;     rsi: pointer to the buffer containing the b64 to decode
;     rdx: pointer to the output buffer. Must be at least 3/4 the size of the input buffer
;   Return value:
;     rax: number of bytes written to the output buffer
; )=== ;
b64_decode:
  ; The algorythm is explained here : https://base64.guru/learn/base64-algorithm/decode.
  ; It is mostly the same here, but I do it in a single loop and with added checks for "=" (padding) and invalid characters
  ;
  ; Internal variables:
  ;   al:   output char (what will be written on the output buffer)
  ;   rbx:  loop counter
  ;   cl:   missing bits (used cl because i need to shift bits with it)
  ;   r8:   index of char in input (called i in comments)
  ;   r9:   index of char in output
  ;   r10b: character (for all the manipulation before it is writable to dil)
  ;   r11:  mask (= (1 << r9) - 1, to extract the first r9 lsb bits of a number, to complete the missing char)

  xor rcx, rcx  ; <- reset registers
  xor r9,  r9   ; <-
  xor r10, r10  ; <-

  ; ---[ for (i = 0; i < input_len - 1; i++) ]--- ;
  xor r8, r8
  .loop:
    cmp cl, 6                ; if (missing_bits != 6)
    jne .missing_bits_not_6   ; don't reset missing bits and decrement i
      xor cl, cl  ; reset missing bits
      inc r8     ; remove 1 from i (skip a char, that was previously entirely used)

    .missing_bits_not_6:

    inc r8           ; <- to check if we aren't going to read too far into input.
    cmp r8, rdi      ; <- compare r8 + 1 (the furthest char we are reading this iteration) and rdi (input len)
    jge .return      ; <- if (current char index + 1 == input_len) { return }
    dec r8

    mov r10b, byte [rsi + r8]       ; load input[char_index]

    cmp r10b, 0x3d     ; if input[char_index] == "="
    je .return         ; we are done (padding)

    mov al, byte [b64_table + r10]   ; translate char according to b64 table

    cmp al, 0xff       ; if the input char is not a valid b64 character
    je .fail           ; we goto the failed branch

    shl al, 2    ; remove useless 00
    shl al, cl   ; remove already used bits

    add cl, 2    ; add 2 to the number of bits required to complete the character

    mov r10b, byte [rsi + r8 + 1]   ; take next char to complete the previous one

    cmp r10b, 0x3d     ; if input[char_index] == "="
    je .return         ; we are done (padding)

    mov r10b, byte [b64_table + r10]  ; translate char according to b64 table

    cmp r10b, 0xff     ; if the input char is not a valid b64 character
    je .fail           ; we goto the failed branch

    ; ↓ mask creation, to keep only the cl (missing bits) first bits of the next char
    mov r11b, 1   ; initialize to 1
    shl r11b, cl  ; shift cl (missing bits) bits
    dec r11b      ; substract 1

    add cl, 2     ; ↓ why can we only do that with cl :'(
    rol r10b, cl  ; move the interesting bits at the beginning
    sub cl, 2

    and r10b, r11b ; remove all the bits we don't care about

    add al, r10b   ; complete the previous character with the missing bits

    mov [rdx + r9], byte al

    inc r9         ; increment index of output char

    inc r8
    cmp r8, rdi
    jb .loop

  .return:
    mov rax, r9
    ret

  .fail:
    xor r9, r9
    jmp .return
; -------------------------------
