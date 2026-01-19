---
tags:
  - tech
  - ctf/reverse
---
In this writeup, I'll be using both binary ninja and rizin for static analysis, since I'm more familiar with rizin, but binary ninja is more mature and is more widely known. You can get basically the same results as rizin with a gui thanks to cutter, but I often work on my potato computer so I appreciate having a more lightweight version avaliable.
I include the commands for my analysis in rizin so that you can follow along if you are not familiar with rizin.

Also this writeup will be pretty lengthy, if you want a shorter version of this writeup, you can check out [this](https://ctf.thaysan.com/ctf-and-writeups/2025-or-hackday/reverse/rusty_rev) writeup or the old one that should still be on the repo.

# Challenge description

Hello agent, a trusted source managed to find one of the most secured app of the black mist crew, but unfortunately, we can't find the password to access it.
We know your talents for reverse engineering, we need you to help us this password.

---
sha256: 71553d736b4299a40069ff3ae1fbd242b50f88b44c28a49ef559ac34248581d5

# First stage

- `file` :
 	- `rusty_rev: ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, BuildID[sha1]=943ca8eb509825e8c496b58e81ec9d3b46b2bd5f, for GNU/Linux 4.4.0, stripped`
 	- We can guess it is a rust based stripped elf executable, gonna be fun
When we run it, we are asked for a password, we can try anything and we get the message : `Wrong password, reporting incident to the admin`
- We can check the strings :
 	- first, we don't see any mention of the string we found when we ran it, maybe the strings are obfuscated, or we have a dropper
 	- we don't get that much informations, but we can find `memfd_create`, `fexecve` and a mention of the aes crate, potentially confirming our loader hypothesis

With `rz-bin -i rusty_rev`, we can get a list of imports, we can see the `memfd_create` and `fexecve` functions, most likely meaning that we have a dropper.
Let's talk about those two functions :

- [memfd_create](https://www.man7.org/linux/man-pages/man2/memfd_create.2.html) : creates an anonymous file in memory and returns its file descriptor.
- [fexecve](https://www.man7.org/linux/man-pages/man3/fexecve.3.html) : replaces the current running process by running the file specified by the file descriptor.
You can guess how those two function can combine to get a first stage that gets the (generally encrypted) payload, decodes it and then run it without the malicious file ever touching the victim's hard drive. It also allows to easily change the key for the payload, completely bypassing signature detection (if the key and payload are pulled from remote rather than hard coded). At first, I wanted to implement this kind of behavior and make the change really frequent, but I figured the challenge was already enough as is (maybe a future challenge, who knows ?).
On windows, you can get the same behavior with different techniques, but I won't dive into it here.

You now have two option : retrieving the payload using static or dynamic analysis.
For your sanity, I would strongly recommend going the dynamic route, as statically reversing stripped rust is might not be a pleasant experience, and why do yourself what the program already does itself.
If you still want to go this route, I won't give you the exact steps, but here is the general overview :

- get the encrypted file and the key (in the `.rodata` section)
- figure out it's aes
- decode it
If someone managed to do it this way, congrats, send me the WU and I'll link it.

Now let's get to actually extracting the payload.
Let's open the executable in gdb. I'm using [gef](https://github.com/hugsy/gef), but you can still follow using pure gdb.
If we run `info func`, we see :

```
All defined functions:

Non-debugging symbols:
0x0000000000006030  __tls_get_addr@plt
0x0000000000006040  _Unwind_Resume@plt
```

Not a lot of functions, let's start the program with `starti`.
If we run `info func` again, we see more functions than before :O, but there's still isn't enough functions, so let's `si 3` then `finish` to let all functions be detected.
Now, we can put a breakpoint on `memfd_create` with `b memfd_create` and go there with `c`. The disassembly of `memfd_create` (with `disass`) :

```asm
=> 0x00007ffff7e86890 <+0>:     endbr64
   0x00007ffff7e86894 <+4>:     mov    eax,0x13f
   0x00007ffff7e86899 <+9>:     syscall
   0x00007ffff7e8689b <+11>: cmp    rax,0xfffffffffffff001
   0x00007ffff7e868a1 <+17>: jae    0x7ffff7e868a4 <memfd_create+20>
   0x00007ffff7e868a3 <+19>: ret
   0x00007ffff7e868a4 <+20>: mov    rcx,QWORD PTR [rip+0xcc445]        # 0x7ffff7f52cf0
   0x00007ffff7e868ab <+27>: neg    eax
   0x00007ffff7e868ad <+29>: mov    DWORD PTR fs:[rcx],eax
   0x00007ffff7e868b0 <+32>: or     rax,0xffffffffffffffff
   0x00007ffff7e868b4 <+36>: ret
```

We can see there's not a lot going on, we see a syscall with `0x13f`, which is `sys_memfd_create` (according to [here](https://blog.rchapman.org/posts/Linux_System_Call_Table_for_x86_64/)), and fair enough, we weren't lied to.
We then compare `rax` (return value of the syscall) to something and if it's above or equal, we don't return yet and do stuff. We can guess it's for error handling, so we don't care about it, and we can see that we do, in fact, return directly after the compare (if not, we crash).
Let's remember the value in `rax`, because we'll need it in a minute (I have 3), but you might have something different

This was kind of short, and we still don't have the payload. Do not worry, if we take a look at where we currently are, we see :

```asm
   0x000055555555bedb: call   QWORD PTR [rip+0xb0acf]        # 0x55555560c9b0
=> 0x000055555555bee1: cmp    eax,0xffffffff
   0x000055555555bee4: jne    0x55555555bf46
```

Yet again error checking, if eax != -1, we goto `0x55555555bf46`, and in there, we see

```asm
=> 0x000055555555bf46: mov    ebp,eax
   0x000055555555bf48: mov    edi,eax
   0x000055555555bf4a: mov    rsi,r12
   0x000055555555bf4d: mov    rdx,r15
   0x000055555555bf50: call   QWORD PTR [rip+0xb0b2a]        # 0x55555560ca80
```

A function call, but to what ? To the `write` function. This is where it becomes interesting, it is `write`, let's check [the documentation](https://www.man7.org/linux/man-pages/man2/write.2.html). The first argument is a file descriptor, the second one is a pointer to the buffer we want to write to the file descriptor (so it's start address) and the size of the buffer.
Let's check the arguments. It's a x86-64 binary, so the arguments are passed with the [System V convention](https://en.wikipedia.org/wiki/X86_calling_conventions#System_V_AMD64_ABI).
Those values will probably be different for you.
The first argument is 3 (the file descriptor, also the return value of `memfd_create` I told you to remember, in `rdi`).
The second one is `0x00007ffff7c48010` (the start of the buffer, in `rsi`).
The last one is `0x5f9c0` (the length of the buffer, in `rdx`).

Cool, we now have everything we need in order to extract what we want. We can dump the payload using `dump memory stage2 0x00007ffff7c48010 0x7ffff7ca79d0`.

Let's check that we correctly extracted it :
We can check the magic bytes to see if it is a elf executable with `xxd stage2 | head`. A quick google search teaches us that the magic for an elf file is `7F 45 4C 46`, and cool, that's what we have.
When we run it, we have the same password prompt and the same error message, it's a good sign.
Here is the sha256sum of the second stage, to check if you have the same one as me : `9339a06656d6f66feefc9d2828b9f8a3845ba843360ed4b29554a6ff2bd92566`.

# Second stage

We should now be able to find the strings we couldn't find before because of the encryption.
If we run `strings stage2 | grep password`, we get :

```
already borrowedassertion `left  right` failed: range end index ) when slicing `core/src/time.rs/rustc/f6e511eec7342f59a25f7c0534f1dbea00d01b14/library/core/src/str/pattern.rscalled `Result::unwrap()` on an `Err` valuePlease input your password : src/main.rsFailed to read lineWrong password, reporting incident to the admin
ggwp, you can validate using this password
```

Appart from the string termination not being correct, we can see that we do find the string we saw before, but we also get a little bonus : `ggwp, you can validate using this password`. This is easy to guess it's the string being displayed when you enter the right password.

The next thing we might want to do is to try and put a breakpoint on `write` and see wich one prints `Please input your password :`, and have a look around to see the password check, so let's do it.
We can see the program doing weird things like :

- printing an among us ASCII art, the most obvious one, intended to not be too evil and give you a clue that there was anti-debugging

```
░░░░░░░░░░░░░░░░█████░░░░░░░░░░░░░░░░░░░
░░░░░░░░░░░░░████░░░░░███░░░░░░░░░░░░░░░
░░░░██████████░░░░░░████████████░░░░░░░░
░░░░█░░░░░░░█░░░░████░░░░░░░░░░██░░░░░░░
░░░██░░░░░░██░░░░░█░░░░░░░░░░░██░░░░░░░░
░░░█░░░░░░░█░░░░░░███░░░░░█████░░░░░░░░░
░░░░█░░░░░░█░░░░░░░░░███████░░░░░░░░░░░░
░░░░░███████░░░░░░░░░░░░░░░█░░░░░░░░░░░░
░░░░░░░░░░░█░░░███████░░░░░█░░░░░░░░░░░░
░░░░░░░░░░░█░░░█░░░░░█░░░░░█░░░░░░░░░░░░
░░░░░░░░░░░█░░░█░░░░░█░░░░█░░░░░░░░░░░░░
░░░░░░░░░░░█░░░█░░░░░█░░░░█░░░░░░░░░░░░░
░░░░░░░░░░░█░░░█░░░░░█░░░░█░░░░░░░░░░░░░
░░░░░░░░░░░█░░░█░░░░░██████░░░░░░░░░░░░░
░░░░░░░░░░░█████░░░░░░░░░░░░░░░░░░░░░░░░
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
```

- printing `gdb error, please reboot the computer` and leaving, still a little bit obvious
- printing `ggwp, you can validate using this password` before checking for password, then leaving. Kind of evil, but more realistic
- replacing the string `5734M3D` with `ST0P17`, we'll see what this means later, very evil.
The most obvious ones were meant to alert you on the anti-debugging, and the most sneaky ones were here to be more realistic.
We don't need to know the specifics of anti-debugging, only know there is some and how to bypass it, so let's find out how to bypass it.

For the remainder of this challenge, I used `cerberus` to unstrip the binary in order to have more descriptive function names and I matched the offsets of the functions in gdb and in binary ninja with rebase.

We are now looking for the anti-debug part. First, let's try to look for the main function in binary ninja. One easy way to find it is to do a string Xref analysis. We can search for `password` :
![[Pasted image 20250213153809.png]]
We find :
![[Pasted image 20250215200548.png]]
![[Pasted image 20250213153850.png]]

We see a function call to `open` with the parameter `/proc/self/status`. This file on linux systems is used to display informations about the current process. Thanks to rust janky string termination, we see appear `TracerPid :\t` right after the file name, which is luckily a line in the file we are opening that gives the pid of the debugger or 0 if it's not being debugged.
We can easily guess how it can be used to detect debugging and annoy reverse engineer.

After that, there is a lot of code, most of which doesn't interest us. We can try and locate the part we care about with a `close` call to the file descriptor we got from the open function earlier.
And there it is :
![[Pasted image 20250213154900.png]]
Nice, if we take a look around, we see strings assignments with a string containing the string we found earlier, containing interesting stuff. Right after, we check for the value of rbx_3 and do other things with the strings.
If we check where `rbx_3` is assigned, we see this function with the arguments 1, 5 and `&var_b8`. We can guess that `var_b8` is a rng generator and `sub_55555555d284` is the generation of this number between 1 and 4 (included) based on the error message here :
![[Pasted image 20250213162918.png]]
![[Pasted image 20250213162936.png]]

And just under it, we see :
![[Pasted image 20250213163151.png]]

Based on what we found out, the first if we see is the being debugged case (there is 4 random event that could happen and we generate a random number between 1 and 4, so that is probably what decides what is the event) and the second one is the not being debugged case, so the one we want without all the annoyance.

With all this info, we can but a breakpoint on `0x55555555ccff` and change the value we got from the random function. We could also try to spoof the `/proc/self/status` file (that would be the best way to do it, because there could be other places where we check it, but there is only a single check, and I'm too lazy to do it) or take the false branch of the being debugged case.

With the anti-debugging finally out of the way, we can go on to find the password. We are asked to input something, so we can put a breakpoint on `read` and skip all the one where we aren't asked to input anything (and all the ones before the patch of the random value). This process allows us to find the address right after the read : `0x55555555cdbd`.
Looking around here in binary ninja, we find this interesting check :
![[Pasted image 20250215155714.png]]
this is probably the length of our input, we can verify this dynamically, and turns out it is indeed the length of our input.
We also see that if it's the right length, we compare `var_110_1+7` to something in `0x5555555A1040` and `var_110_1` to something in `0x5555555A1050`. If we check what's in there, we get : `a5 e7 f3 3f 8d e1 f5` and `0f ca 1b 12 65 0f e1 05 22 0f cf 14 36 e8 3c 3a`. If we concatenate them, we find something of length 23 (0x17), which is also suspiciously the length of the password we need to find.
With that in mind, we can start to test different inputs. With only A, we get :

```
gef➤  x/23b $rbx
0x5555555b7c00: 0xe4 0xe7 0xe1 0x65 0xa0 0xe1 0x2d 0xe4
0x5555555b7c08: 0xe7 0xe1 0x65 0xa0 0xe1 0x2d 0xe4 0xe7
0x5555555b7c10: 0xe1 0x65 0xa0 0xe1 0x2d 0xe4 0xe7
```

We can see it is cyclic, the byte we see at index 0 is the same as the one in index 7. I'll explain why that is a little bit after.
We can do it with all letters (you can do it manually or script it, since I knew which letters I had to do, it was faster to do it manually), and you'll get a mask for each possible letter with its possible values and their index. With that, we can do a small script to match the corresponding letters and get the flag. Here is a little exemple :

```python
expected = b"\xa5\xe7\xf3\x3f\x8d\xe1\xf5\x0f\xca\x1b\x12\x65\x0f\xe1\x05\x22\x0f\xcf\x14\x36\xe8\x3c\x3a"
dico = {
    "0": b"\x1e\x1d\x1b\x9f\x5a\x1b\xd7\x1e\x1d\x1b\x9f\x5a\x1b\xd7\x1e\x1d\x1b\x9f\x5a\x1b\xd7\x1e\x1d",
    "3": b"\x05\x06\x00\x84\x41\x00\xcc\x05\x06\x00\x84\x41\x00\xcc\x05\x06\x00\x84\x41\x00\xcc\x05\x06",
    "5": b"\x33\x30\x36\xb2\x77\x36\xfa\x33\x30\x36\xb2\x77\x36\xfa\x33\x30\x36\xb2\x77\x36\xfa\x33\x30",
    "6": b"\x28\x2b\x2d\xa9\x6c\x2d\xe1\x28\x2b\x2d\xa9\x6c\x2d\xe1\x28\x2b\x2d\xa9\x6c\x2d\xe1\x28\x2b",
    "7": b"\x21\x22\x24\xa0\x65\x24\xe8\x21\x22\x24\xa0\x65\x24\xe8\x21\x22\x24\xa0\x65\x24\xe8\x21\x22",
    "A": b"\xe4\xe7\xe1\x65\xa0\xe1\x2d\xe4\xe7\xe1\x65\xa0\xe1\x2d\xe4\xe7\xe1\x65\xa0\xe1\x2d\xe4\xe7",
    "C": b"\xf6\xf5\xf3\x77\xb2\xf3\x3f\xf6\xf5\xf3\x77\xb2\xf3\x3f\xf6\xf5\xf3\x77\xb2\xf3\x3f\xf6\xf5",
    "D": b"\xc9\xca\xcc\x48\x8d\xcc\x00\xc9\xca\xcc\x48\x8d\xcc\x00\xc9\xca\xcc\x48\x8d\xcc\x00\xc9\xca",
    "H": b"\xa5\xa6\xa0\x24\xe1\xa0\x6c\xa5\xa6\xa0\x24\xe1\xa0\x6c\xa5\xa6\xa0\x24\xe1\xa0\x6c\xa5\xa6",
    "K": b"\xbe\xbd\xbb\x3f\xfa\xbb\x77\xbe\xbd\xbb\x3f\xfa\xbb\x77\xbe\xbd\xbb\x3f\xfa\xbb\x77\xbe\xbd",
    "N": b"\x93\x90\x96\x12\xd7\x96\x5a\x93\x90\x96\x12\xd7\x96\x5a\x93\x90\x96\x12\xd7\x96\x5a\x93\x90",
    "U": b"\x50\x53\x55\xd1\x14\x55\x99\x50\x53\x55\xd1\x14\x55\x99\x50\x53\x55\xd1\x14\x55\x99\x50\x53",
    "Y": b"\x3c\x3f\x39\xbd\x78\x39\xf5\x3c\x3f\x39\xbd\x78\x39\xf5\x3c\x3f\x39\xbd\x78\x39\xf5\x3c\x3f",
    "r": b"\x4e\x4d\x4b\xcf\x0a\x4b\x87\x4e\x4d\x4b\xcf\x0a\x4b\x87\x4e\x4d\x4b\xcf\x0a\x4b\x87\x4e\x4d",
    "_": b"\x0a\x09\x0f\x8b\x4e\x0f\xc3\x0a\x09\x0f\x8b\x4e\x0f\xc3\x0a\x09\x0f\x8b\x4e\x0f\xc3\x0a\x09",
    "{": b"\x0f\x0c\x0a\x8e\x4b\x0a\xc6\x0f\x0c\x0a\x8e\x4b\x0a\xc6\x0f\x0c\x0a\x8e\x4b\x0a\xc6\x0f\x0c",
    "}": b"\x39\x3a\x3c\xb8\x7d\x3c\xf0\x39\x3a\x3c\xb8\x7d\x3c\xf0\x39\x3a\x3c\xb8\x7d\x3c\xf0\x39\x3a",
}

print(dico.keys())
output_str = ""
for i in range(len(expected)):
    for key in dico:
        if expected[i] == dico[key][i]:
            print(f"---CHAR {i} is {key}")
            output_str += key
            break
    else:
        output_str += "X"
print(output_str)
```

I only included the letters needed to solve because it was easier and more legible, but in a real case, all possible letters would be here, and there it is, the flag : `HACKDAY{D0N7_637_rU57Y}`.

Let's talk a little bit more about what we just did. If you only care about how to solve the chall, you can skip.

Remember the 4th anti-debug option I said we'll talk about later ? Later is now. For fun, test setting `eax` to 1 at `0x55555555ccff` and do the same process of inputting a letter and checking the result.
Funny how it isn't the same thing. That's because the string `5734M3D` is used to "encrypt" your input, and like I said earlier, I made it change in this case.
The "encryption" algorithm is `(x1.rotate_left(3) ^ x1) ^ (x2.rotate_right(1) ^ x2)` with `x1` the chars of your input and `x2` the chars of the key.

I gave you the original code, if you want to check out what it looks like not compiled. I find reversing at least one or two rust program very educating, as we can see loop unwinding and all the optimisations that rust makes that decompiler sometimes struggle to understand.

That's it, hope you enjoyed my challenge, if you did, I appreciate it; if you didn't, let me know why.

