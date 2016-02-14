# rappel

Rappel is a pretty janky assembly REPL. It works by creating a shell ELF, starting it under ptrace, then continiously rewriting/running the `.text` section, while showing the register states. It's maybe half done right now, and supports Linux x86, amd64, and armv7 (no thumb) at the moment.

## Install

The only dependencies are libedit and nasm, which on debian can be installed with the `libedit-dev` and `nasm` packages.

```
$ CC=clang make
```

It should work fine with `gcc`, albiet with a few more warnings.

By default rappel is compiled with your native architecture. If you're on amd64 and want to target x86 you can do this with

```
$ ARCH=i386 CC=clang make
```

In theory you can also compile an armv7 binary this way, but I really doubt it will work. For rappel to function, the architecture of the main rappel binary must match that of the process it creates, and the host must be able to run binaries of this architecture.

## Running

Rappel has two modes it can operate in. A pipe mode for one off things, a la

```
$ echo "inc eax" | bin/rappel
rax:0x0000000000000001  rbx:0x0000000000000000  rcx:0x0000000000000000  rdx:0x0000000000000000
rsi:0x0000000000000000  rdi:0x0000000000000000  r8 :0x0000000000000000  r9 :0x0000000000000000
r10:0x0000000000000000  r11:0x0000000000000000  r12:0x0000000000000000  r13:0x0000000000000000
r14:0x0000000000000000  r15:0x0000000000000000
rip:0x0000000000400003  rsp:0x00007ffd7da9f820  rbp:0x0000000000000000
flags:0x0000000000000202
$
```

Or an interactive mode:

```
$ bin/rappel
rax:0x0000000000000000  rbx:0x0000000000000000  rcx:0x0000000000000000  rdx:0x0000000000000000
rsi:0x0000000000000000  rdi:0x0000000000000000  r8 :0x0000000000000000  r9 :0x0000000000000000
r10:0x0000000000000000  r11:0x0000000000000000  r12:0x0000000000000000  r13:0x0000000000000000
r14:0x0000000000000000  r15:0x0000000000000000
rip:0x0000000000400001  rsp:0x00007fff9b172670  rbp:0x0000000000000000
flags:0x0000000000000202
> inc rax
rax:0x0000000000000001  rbx:0x0000000000000000  rcx:0x0000000000000000  rdx:0x0000000000000000
rsi:0x0000000000000000  rdi:0x0000000000000000  r8 :0x0000000000000000  r9 :0x0000000000000000
r10:0x0000000000000000  r11:0x0000000000000000  r12:0x0000000000000000  r13:0x0000000000000000
r14:0x0000000000000000  r15:0x0000000000000000
rip:0x0000000000400004  rsp:0x00007fff9b172670  rbp:0x0000000000000000
flags:0x0000000000000202
> push rax
rax:0x0000000000000001  rbx:0x0000000000000000  rcx:0x0000000000000000  rdx:0x0000000000000000
rsi:0x0000000000000000  rdi:0x0000000000000000  r8 :0x0000000000000000  r9 :0x0000000000000000
r10:0x0000000000000000  r11:0x0000000000000000  r12:0x0000000000000000  r13:0x0000000000000000
r14:0x0000000000000000  r15:0x0000000000000000
rip:0x0000000000400002  rsp:0x00007fff9b172668  rbp:0x0000000000000000
flags:0x0000000000000202
> pop rbx
rax:0x0000000000000001  rbx:0x0000000000000001  rcx:0x0000000000000000  rdx:0x0000000000000000
rsi:0x0000000000000000  rdi:0x0000000000000000  r8 :0x0000000000000000  r9 :0x0000000000000000
r10:0x0000000000000000  r11:0x0000000000000000  r12:0x0000000000000000  r13:0x0000000000000000
r14:0x0000000000000000  r15:0x0000000000000000
rip:0x0000000000400002  rsp:0x00007fff9b172670  rbp:0x0000000000000000
flags:0x0000000000000202
> cmp rax, rbx
rax:0x0000000000000001  rbx:0x0000000000000001  rcx:0x0000000000000000  rdx:0x0000000000000000
rsi:0x0000000000000000  rdi:0x0000000000000000  r8 :0x0000000000000000  r9 :0x0000000000000000
r10:0x0000000000000000  r11:0x0000000000000000  r12:0x0000000000000000  r13:0x0000000000000000
r14:0x0000000000000000  r15:0x0000000000000000
rip:0x0000000000400004  rsp:0x00007fff9b172670  rbp:0x0000000000000000
flags:0x0000000000000246
> ^D
$
```

## Docs

You can get pretty much all the documentation with either `-h` from the command line, or `.help` from the interactive bit.