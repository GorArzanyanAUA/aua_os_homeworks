# Homework 4 — Object Files, Symbols and Linking

**Repository:** https://github.com/GorArzanyanAUA/aua_os_homeworks

**Author:** Gor Arzanyan

## The program

Three files: `main.c` holds `main` and calls `square_num`; `math_utils.c` implements
it; `math_utils.h` declares it.

```c
// math_utils.h
int square_num(int num);
```

Built in two stages, so that the intermediate object files can be inspected:

```
gcc -c main.c           # -> main.o
gcc -c math_utils.c     # -> math_utils.o
gcc main.o math_utils.o -o square_prog
./square_prog           # x : 10 ; x^2 : 100
```


---

## 1. What symbols are defined in each file?

`nm` prints one line per symbol: an address (blank if there is none yet), a one-letter
type code, and the name.

| code | meaning |
|---|---|
| `T` | defined in `.text` (code), globally visible |
| `t` | defined in `.text`, but local to the file |
| `U` | **undefined** — referenced here, must be supplied by someone else |
| `D` / `B` / `R` | defined in `.data` / `.bss` / `.rodata` |
| `W` / `w` | weak — may be left unresolved without an error |

### `main.o`

```
0000000000000000 T main
                 U printf
                 U square_num
```

One definition, `main`, and two debts. `main.o` knows *that* `square_num` and `printf`
exist and what to pass them, because it saw their declarations, but it does not contain
a single byte of either.

### `math_utils.o`

```
0000000000000000 T square_num
```

One definition and no undefined symbols at all.

Note that both files claim address `0000000000000000` for their definitions. Each `.o` is written
as if it owned the address space starting at zero, and choosing the real addresses is the
linker's job.

### `square_prog`

The executable contains far more than the two symbols.

```
0000000000001149 T main
0000000000001188 T square_num
                 U printf@GLIBC_2.2.5
```

Two things changed and one did not:

- **`main` and `square_num` now have real, distinct virtual addresses** (`0x1149` and
  `0x1188`).
- **`printf` is *still* undefined.** It only gained a version tag, `@GLIBC_2.2.5`.  `printf`'s code is not in this file; it
  will be located in `libc.so` by the dynamic loader at program start.

The rest of the symbol table is startup and shutdown machinery that the linker added
automatically:

```
0000000000001060 T _start
                 U __libc_start_main@GLIBC_2.34
0000000000001000 T _init
000000000000119c T _fini
0000000000001140 t frame_dummy
0000000000001100 t __do_global_dtors_aux
00000000000010c0 t register_tm_clones
0000000000001090 t deregister_tm_clones
0000000000004010 B __bss_start
0000000000004018 B _end
                 w __gmon_start__
```

The important one is `_start`, and the ELF header confirms its role:

```
Entry point address:               0x1060
```

`0x1060` is `_start`, **not** `main`. The kernel never calls `main` directly. It jumps to
`_start`, which sets up the stack and hands `main`'s address to `__libc_start_main`:

```
    1078:	48 8d 3d ca 00 00 00 	lea    0xca(%rip),%rdi        # 1149 <main>
    107f:	ff 15 53 2f 00 00    	call   *0x2f53(%rip)          # __libc_start_main
```

`__libc_start_main` initialises the C library, calls `main`.


---

## 2. How the sections differ

### File type — the difference behind all the others

```
main.o        Type: REL (Relocatable file)            Entry point address: 0x0
math_utils.o  Type: REL (Relocatable file)            Entry point address: 0x0
square_prog   Type: DYN (Position-Independent Executable file)  Entry point: 0x1060
```

And, decisively:

```
main.o / math_utils.o    Number of program headers:  0
square_prog              Number of program headers:  13
```

**Section headers** describe the file for the *linker*: here is `.text`, here is
`.rodata`. **Program headers** (segments) describe the file for the *kernel*: map this
byte range at this address with these permissions. An object file has zero program
headers, which is the structural reason it cannot be executed — nothing in it tells the
kernel how to load it. `square_prog` has 13, so it can be run.

---

## 3. The role of the linking process

### The problem

The compiler only ever looks at one source file at a time. It sees `main.c` call
`square_num`, and the header says the call is legal — but it has no idea *where*
`square_num` will sit in memory, because that function lives in a file it never read. So
it writes down the call instruction and leaves the address blank.

The same happens for `printf`, and for the address of the format string. The compiler then
keeps a list of where those blanks are and what belongs in each. An object file is
therefore an unfinished thing: correct instructions with deliberate holes in them, plus a
list of repairs for somebody else to carry out. Run it as it stands and it would crash.

### What the linker did

1. **Matched up the blanks.** `main.o` needs `square_num`, and `math_utils.o` provides it
   — match.
2. **Decided where everything goes.** Each object file is written as though it were the
   only thing in memory, starting from address zero. The linker stacked them into a single
   layout and grouped them by permission: code in one region, read-only data in another,
   writable data in a third. Only now does each function have a real, unique address.
3. **Filled in the blanks**, which is only possible once step 2 has settled the addresses.
   Each recorded hole gets the number that makes the instruction reach its target.
4. **Added the startup code.** Our program contains no code to set up the stack or to
   start the C library, so the linker supplied it from the C runtime files, and described
   the result in a form the kernel can load. This is why execution actually begins in that
   startup code rather than in `main` — `main` is called by it.

### The exception: library functions

`square_num` was **copied into** the executable. `printf` was **not** — it stays in the
shared C library, since copying it into every program that prints anything would waste
both disk and memory.

So the linker could not fill that blank at all. Instead it pointed the call at a small stub
inside our program, which jumps to whatever address is stored in a nearby table slot. That
slot is left empty on disk. When the program starts, the dynamic loader finds the real
`printf` in the C library and writes its address into the slot, and from then on the jump
lands in the right place.

This is the essential division: `square_num` was resolved at **link time** — copied in and
its call patched to a fixed address — while `printf` is resolved at **run time**, by the
loader, every time the program runs.

---
## Conclusion

An object file is a fragment: correct instructions with deliberate holes, every symbol at
address zero, no program headers, and a list of repairs for someone else to perform. The
three tools each expose one facet of the same gap — `nm` shows it as undefined symbols,
`readelf` as the `.rela.text` section and the absent program headers, `objdump` as literal
zero bytes in the call instructions. Linking closes the gap: it resolves names to
definitions, assigns real addresses, patches the holes, adds the startup code that makes
the file runnable, and defers to the dynamic loader whatever lives in a shared library.

---

# Appendix — terminal output

All commands run in `~/aua_os_homeworks/hw4`.

## `nm`

### `nm main.o`

```
0000000000000000 T main
                 U printf
                 U square_num
```

### `nm math_utils.o`

```
0000000000000000 T square_num
```

### `nm square_prog`

```
000000000000038c r __abi_tag
0000000000004010 B __bss_start
0000000000004010 b completed.0
                 w __cxa_finalize@GLIBC_2.2.5
0000000000004000 D __data_start
0000000000004000 W data_start
0000000000001090 t deregister_tm_clones
0000000000001100 t __do_global_dtors_aux
0000000000003dc0 d __do_global_dtors_aux_fini_array_entry
0000000000004008 D __dso_handle
0000000000003dc8 d _DYNAMIC
0000000000004010 D _edata
0000000000004018 B _end
000000000000119c T _fini
0000000000001140 t frame_dummy
0000000000003db8 d __frame_dummy_init_array_entry
0000000000002120 r __FRAME_END__
0000000000003fb8 d _GLOBAL_OFFSET_TABLE_
                 w __gmon_start__
0000000000002018 r __GNU_EH_FRAME_HDR
0000000000001000 T _init
0000000000002000 R _IO_stdin_used
                 w _ITM_deregisterTMCloneTable
                 w _ITM_registerTMCloneTable
                 U __libc_start_main@GLIBC_2.34
0000000000001149 T main
                 U printf@GLIBC_2.2.5
00000000000010c0 t register_tm_clones
0000000000001188 T square_num
0000000000001060 T _start
0000000000004010 D __TMC_END__
```

## `objdump -d`

### `objdump -d main.o`

```
main.o:     file format elf64-x86-64


Disassembly of section .text:

0000000000000000 <main>:
   0:	f3 0f 1e fa          	endbr64
   4:	55                   	push   %rbp
   5:	48 89 e5             	mov    %rsp,%rbp
   8:	48 83 ec 10          	sub    $0x10,%rsp
   c:	c7 45 fc 0a 00 00 00 	movl   $0xa,-0x4(%rbp)
  13:	8b 45 fc             	mov    -0x4(%rbp),%eax
  16:	89 c7                	mov    %eax,%edi
  18:	e8 00 00 00 00       	call   1d <main+0x1d>
  1d:	89 c2                	mov    %eax,%edx
  1f:	8b 45 fc             	mov    -0x4(%rbp),%eax
  22:	89 c6                	mov    %eax,%esi
  24:	48 8d 05 00 00 00 00 	lea    0x0(%rip),%rax        # 2b <main+0x2b>
  2b:	48 89 c7             	mov    %rax,%rdi
  2e:	b8 00 00 00 00       	mov    $0x0,%eax
  33:	e8 00 00 00 00       	call   38 <main+0x38>
  38:	b8 00 00 00 00       	mov    $0x0,%eax
  3d:	c9                   	leave
  3e:	c3                   	ret
```

### `objdump -d math_utils.o`

```
math_utils.o:     file format elf64-x86-64


Disassembly of section .text:

0000000000000000 <square_num>:
   0:	f3 0f 1e fa          	endbr64
   4:	55                   	push   %rbp
   5:	48 89 e5             	mov    %rsp,%rbp
   8:	89 7d fc             	mov    %edi,-0x4(%rbp)
   b:	8b 45 fc             	mov    -0x4(%rbp),%eax
   e:	0f af c0             	imul   %eax,%eax
  11:	5d                   	pop    %rbp
  12:	c3                   	ret
```

### `objdump -d square_prog`

```
square_prog:     file format elf64-x86-64


Disassembly of section .init:

0000000000001000 <_init>:
    1000:	f3 0f 1e fa          	endbr64
    1004:	48 83 ec 08          	sub    $0x8,%rsp
    1008:	48 8b 05 d9 2f 00 00 	mov    0x2fd9(%rip),%rax        # 3fe8 <__gmon_start__@Base>
    100f:	48 85 c0             	test   %rax,%rax
    1012:	74 02                	je     1016 <_init+0x16>
    1014:	ff d0                	call   *%rax
    1016:	48 83 c4 08          	add    $0x8,%rsp
    101a:	c3                   	ret

Disassembly of section .plt:

0000000000001020 <.plt>:
    1020:	ff 35 9a 2f 00 00    	push   0x2f9a(%rip)        # 3fc0 <_GLOBAL_OFFSET_TABLE_+0x8>
    1026:	ff 25 9c 2f 00 00    	jmp    *0x2f9c(%rip)        # 3fc8 <_GLOBAL_OFFSET_TABLE_+0x10>
    102c:	0f 1f 40 00          	nopl   0x0(%rax)
    1030:	f3 0f 1e fa          	endbr64
    1034:	68 00 00 00 00       	push   $0x0
    1039:	e9 e2 ff ff ff       	jmp    1020 <_init+0x20>
    103e:	66 90                	xchg   %ax,%ax

Disassembly of section .plt.got:

0000000000001040 <__cxa_finalize@plt>:
    1040:	f3 0f 1e fa          	endbr64
    1044:	ff 25 ae 2f 00 00    	jmp    *0x2fae(%rip)        # 3ff8 <__cxa_finalize@GLIBC_2.2.5>
    104a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)

Disassembly of section .plt.sec:

0000000000001050 <printf@plt>:
    1050:	f3 0f 1e fa          	endbr64
    1054:	ff 25 76 2f 00 00    	jmp    *0x2f76(%rip)        # 3fd0 <printf@GLIBC_2.2.5>
    105a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)

Disassembly of section .text:

0000000000001060 <_start>:
    1060:	f3 0f 1e fa          	endbr64
    1064:	31 ed                	xor    %ebp,%ebp
    1066:	49 89 d1             	mov    %rdx,%r9
    1069:	5e                   	pop    %rsi
    106a:	48 89 e2             	mov    %rsp,%rdx
    106d:	48 83 e4 f0          	and    $0xfffffffffffffff0,%rsp
    1071:	50                   	push   %rax
    1072:	54                   	push   %rsp
    1073:	45 31 c0             	xor    %r8d,%r8d
    1076:	31 c9                	xor    %ecx,%ecx
    1078:	48 8d 3d ca 00 00 00 	lea    0xca(%rip),%rdi        # 1149 <main>
    107f:	ff 15 53 2f 00 00    	call   *0x2f53(%rip)        # 3fd8 <__libc_start_main@GLIBC_2.34>
    1085:	f4                   	hlt
    1086:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    108d:	00 00 00

0000000000001090 <deregister_tm_clones>:
    1090:	48 8d 3d 79 2f 00 00 	lea    0x2f79(%rip),%rdi        # 4010 <__TMC_END__>
    1097:	48 8d 05 72 2f 00 00 	lea    0x2f72(%rip),%rax        # 4010 <__TMC_END__>
    109e:	48 39 f8             	cmp    %rdi,%rax
    10a1:	74 15                	je     10b8 <deregister_tm_clones+0x28>
    10a3:	48 8b 05 36 2f 00 00 	mov    0x2f36(%rip),%rax        # 3fe0 <_ITM_deregisterTMCloneTable@Base>
    10aa:	48 85 c0             	test   %rax,%rax
    10ad:	74 09                	je     10b8 <deregister_tm_clones+0x28>
    10af:	ff e0                	jmp    *%rax
    10b1:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    10b8:	c3                   	ret
    10b9:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)

00000000000010c0 <register_tm_clones>:
    10c0:	48 8d 3d 49 2f 00 00 	lea    0x2f49(%rip),%rdi        # 4010 <__TMC_END__>
    10c7:	48 8d 35 42 2f 00 00 	lea    0x2f42(%rip),%rsi        # 4010 <__TMC_END__>
    10ce:	48 29 fe             	sub    %rdi,%rsi
    10d1:	48 89 f0             	mov    %rsi,%rax
    10d4:	48 c1 ee 3f          	shr    $0x3f,%rsi
    10d8:	48 c1 f8 03          	sar    $0x3,%rax
    10dc:	48 01 c6             	add    %rax,%rsi
    10df:	48 d1 fe             	sar    $1,%rsi
    10e2:	74 14                	je     10f8 <register_tm_clones+0x38>
    10e4:	48 8b 05 05 2f 00 00 	mov    0x2f05(%rip),%rax        # 3ff0 <_ITM_registerTMCloneTable@Base>
    10eb:	48 85 c0             	test   %rax,%rax
    10ee:	74 08                	je     10f8 <register_tm_clones+0x38>
    10f0:	ff e0                	jmp    *%rax
    10f2:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
    10f8:	c3                   	ret
    10f9:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)

0000000000001100 <__do_global_dtors_aux>:
    1100:	f3 0f 1e fa          	endbr64
    1104:	80 3d 05 2f 00 00 00 	cmpb   $0x0,0x2f05(%rip)        # 4010 <__TMC_END__>
    110b:	75 2b                	jne    1138 <__do_global_dtors_aux+0x38>
    110d:	55                   	push   %rbp
    110e:	48 83 3d e2 2e 00 00 	cmpq   $0x0,0x2ee2(%rip)        # 3ff8 <__cxa_finalize@GLIBC_2.2.5>
    1115:	00
    1116:	48 89 e5             	mov    %rsp,%rbp
    1119:	74 0c                	je     1127 <__do_global_dtors_aux+0x27>
    111b:	48 8b 3d e6 2e 00 00 	mov    0x2ee6(%rip),%rdi        # 4008 <__dso_handle>
    1122:	e8 19 ff ff ff       	call   1040 <__cxa_finalize@plt>
    1127:	e8 64 ff ff ff       	call   1090 <deregister_tm_clones>
    112c:	c6 05 dd 2e 00 00 01 	movb   $0x1,0x2edd(%rip)        # 4010 <__TMC_END__>
    1133:	5d                   	pop    %rbp
    1134:	c3                   	ret
    1135:	0f 1f 00             	nopl   (%rax)
    1138:	c3                   	ret
    1139:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)

0000000000001140 <frame_dummy>:
    1140:	f3 0f 1e fa          	endbr64
    1144:	e9 77 ff ff ff       	jmp    10c0 <register_tm_clones>

0000000000001149 <main>:
    1149:	f3 0f 1e fa          	endbr64
    114d:	55                   	push   %rbp
    114e:	48 89 e5             	mov    %rsp,%rbp
    1151:	48 83 ec 10          	sub    $0x10,%rsp
    1155:	c7 45 fc 0a 00 00 00 	movl   $0xa,-0x4(%rbp)
    115c:	8b 45 fc             	mov    -0x4(%rbp),%eax
    115f:	89 c7                	mov    %eax,%edi
    1161:	e8 22 00 00 00       	call   1188 <square_num>
    1166:	89 c2                	mov    %eax,%edx
    1168:	8b 45 fc             	mov    -0x4(%rbp),%eax
    116b:	89 c6                	mov    %eax,%esi
    116d:	48 8d 05 90 0e 00 00 	lea    0xe90(%rip),%rax        # 2004 <_IO_stdin_used+0x4>
    1174:	48 89 c7             	mov    %rax,%rdi
    1177:	b8 00 00 00 00       	mov    $0x0,%eax
    117c:	e8 cf fe ff ff       	call   1050 <printf@plt>
    1181:	b8 00 00 00 00       	mov    $0x0,%eax
    1186:	c9                   	leave
    1187:	c3                   	ret

0000000000001188 <square_num>:
    1188:	f3 0f 1e fa          	endbr64
    118c:	55                   	push   %rbp
    118d:	48 89 e5             	mov    %rsp,%rbp
    1190:	89 7d fc             	mov    %edi,-0x4(%rbp)
    1193:	8b 45 fc             	mov    -0x4(%rbp),%eax
    1196:	0f af c0             	imul   %eax,%eax
    1199:	5d                   	pop    %rbp
    119a:	c3                   	ret

Disassembly of section .fini:

000000000000119c <_fini>:
    119c:	f3 0f 1e fa          	endbr64
    11a0:	48 83 ec 08          	sub    $0x8,%rsp
    11a4:	48 83 c4 08          	add    $0x8,%rsp
    11a8:	c3                   	ret
```

## `readelf -S` (section headers)

### `readelf -S main.o`

```
There are 14 section headers, starting at offset 0x2b8:

Section Headers:
  [Nr] Name              Type             Address           Offset
       Size              EntSize          Flags  Link  Info  Align
  [ 0]                   NULL             0000000000000000  00000000
       0000000000000000  0000000000000000           0     0     0
  [ 1] .text             PROGBITS         0000000000000000  00000040
       000000000000003f  0000000000000000  AX       0     0     1
  [ 2] .rela.text        RELA             0000000000000000  000001e0
       0000000000000048  0000000000000018   I      11     1     8
  [ 3] .data             PROGBITS         0000000000000000  0000007f
       0000000000000000  0000000000000000  WA       0     0     1
  [ 4] .bss              NOBITS           0000000000000000  0000007f
       0000000000000000  0000000000000000  WA       0     0     1
  [ 5] .rodata           PROGBITS         0000000000000000  0000007f
       0000000000000011  0000000000000000   A       0     0     1
  [ 6] .comment          PROGBITS         0000000000000000  00000090
       000000000000002e  0000000000000001  MS       0     0     1
  [ 7] .note.GNU-stack   PROGBITS         0000000000000000  000000be
       0000000000000000  0000000000000000           0     0     1
  [ 8] .note.gnu.pr[...] NOTE             0000000000000000  000000c0
       0000000000000020  0000000000000000   A       0     0     8
  [ 9] .eh_frame         PROGBITS         0000000000000000  000000e0
       0000000000000038  0000000000000000   A       0     0     8
  [10] .rela.eh_frame    RELA             0000000000000000  00000228
       0000000000000018  0000000000000018   I      11     9     8
  [11] .symtab           SYMTAB           0000000000000000  00000118
       00000000000000a8  0000000000000018          12     4     8
  [12] .strtab           STRTAB           0000000000000000  000001c0
       000000000000001f  0000000000000000           0     0     1
  [13] .shstrtab         STRTAB           0000000000000000  00000240
       0000000000000074  0000000000000000           0     0     1
Key to Flags:
  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
  L (link order), O (extra OS processing required), G (group), T (TLS),
  C (compressed), x (unknown), o (OS specific), E (exclude),
  D (mbind), l (large), p (processor specific)
```

### `readelf -S math_utils.o`

```
There are 12 section headers, starting at offset 0x1e0:

Section Headers:
  [Nr] Name              Type             Address           Offset
       Size              EntSize          Flags  Link  Info  Align
  [ 0]                   NULL             0000000000000000  00000000
       0000000000000000  0000000000000000           0     0     0
  [ 1] .text             PROGBITS         0000000000000000  00000040
       0000000000000013  0000000000000000  AX       0     0     1
  [ 2] .data             PROGBITS         0000000000000000  00000053
       0000000000000000  0000000000000000  WA       0     0     1
  [ 3] .bss              NOBITS           0000000000000000  00000053
       0000000000000000  0000000000000000  WA       0     0     1
  [ 4] .comment          PROGBITS         0000000000000000  00000053
       000000000000002e  0000000000000001  MS       0     0     1
  [ 5] .note.GNU-stack   PROGBITS         0000000000000000  00000081
       0000000000000000  0000000000000000           0     0     1
  [ 6] .note.gnu.pr[...] NOTE             0000000000000000  00000088
       0000000000000020  0000000000000000   A       0     0     8
  [ 7] .eh_frame         PROGBITS         0000000000000000  000000a8
       0000000000000038  0000000000000000   A       0     0     8
  [ 8] .rela.eh_frame    RELA             0000000000000000  00000160
       0000000000000018  0000000000000018   I       9     7     8
  [ 9] .symtab           SYMTAB           0000000000000000  000000e0
       0000000000000060  0000000000000018          10     3     8
  [10] .strtab           STRTAB           0000000000000000  00000140
       0000000000000019  0000000000000000           0     0     1
  [11] .shstrtab         STRTAB           0000000000000000  00000178
       0000000000000067  0000000000000000           0     0     1
Key to Flags:
  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
  L (link order), O (extra OS processing required), G (group), T (TLS),
  C (compressed), x (unknown), o (OS specific), E (exclude),
  D (mbind), l (large), p (processor specific)
```

### `readelf -S square_prog`

```
There are 31 section headers, starting at offset 0x36e0:

Section Headers:
  [Nr] Name              Type             Address           Offset
       Size              EntSize          Flags  Link  Info  Align
  [ 0]                   NULL             0000000000000000  00000000
       0000000000000000  0000000000000000           0     0     0
  [ 1] .interp           PROGBITS         0000000000000318  00000318
       000000000000001c  0000000000000000   A       0     0     1
  [ 2] .note.gnu.pr[...] NOTE             0000000000000338  00000338
       0000000000000030  0000000000000000   A       0     0     8
  [ 3] .note.gnu.bu[...] NOTE             0000000000000368  00000368
       0000000000000024  0000000000000000   A       0     0     4
  [ 4] .note.ABI-tag     NOTE             000000000000038c  0000038c
       0000000000000020  0000000000000000   A       0     0     4
  [ 5] .gnu.hash         GNU_HASH         00000000000003b0  000003b0
       0000000000000024  0000000000000000   A       6     0     8
  [ 6] .dynsym           DYNSYM           00000000000003d8  000003d8
       00000000000000a8  0000000000000018   A       7     1     8
  [ 7] .dynstr           STRTAB           0000000000000480  00000480
       000000000000008f  0000000000000000   A       0     0     1
  [ 8] .gnu.version      VERSYM           0000000000000510  00000510
       000000000000000e  0000000000000002   A       6     0     2
  [ 9] .gnu.version_r    VERNEED          0000000000000520  00000520
       0000000000000030  0000000000000000   A       7     1     8
  [10] .rela.dyn         RELA             0000000000000550  00000550
       00000000000000c0  0000000000000018   A       6     0     8
  [11] .rela.plt         RELA             0000000000000610  00000610
       0000000000000018  0000000000000018  AI       6    24     8
  [12] .init             PROGBITS         0000000000001000  00001000
       000000000000001b  0000000000000000  AX       0     0     4
  [13] .plt              PROGBITS         0000000000001020  00001020
       0000000000000020  0000000000000010  AX       0     0     16
  [14] .plt.got          PROGBITS         0000000000001040  00001040
       0000000000000010  0000000000000010  AX       0     0     16
  [15] .plt.sec          PROGBITS         0000000000001050  00001050
       0000000000000010  0000000000000010  AX       0     0     16
  [16] .text             PROGBITS         0000000000001060  00001060
       000000000000013b  0000000000000000  AX       0     0     16
  [17] .fini             PROGBITS         000000000000119c  0000119c
       000000000000000d  0000000000000000  AX       0     0     4
  [18] .rodata           PROGBITS         0000000000002000  00002000
       0000000000000015  0000000000000000   A       0     0     4
  [19] .eh_frame_hdr     PROGBITS         0000000000002018  00002018
       000000000000003c  0000000000000000   A       0     0     4
  [20] .eh_frame         PROGBITS         0000000000002058  00002058
       00000000000000cc  0000000000000000   A       0     0     8
  [21] .init_array       INIT_ARRAY       0000000000003db8  00002db8
       0000000000000008  0000000000000008  WA       0     0     8
  [22] .fini_array       FINI_ARRAY       0000000000003dc0  00002dc0
       0000000000000008  0000000000000008  WA       0     0     8
  [23] .dynamic          DYNAMIC          0000000000003dc8  00002dc8
       00000000000001f0  0000000000000010  WA       7     0     8
  [24] .got              PROGBITS         0000000000003fb8  00002fb8
       0000000000000048  0000000000000008  WA       0     0     8
  [25] .data             PROGBITS         0000000000004000  00003000
       0000000000000010  0000000000000000  WA       0     0     8
  [26] .bss              NOBITS           0000000000004010  00003010
       0000000000000008  0000000000000000  WA       0     0     1
  [27] .comment          PROGBITS         0000000000000000  00003010
       000000000000002d  0000000000000001  MS       0     0     1
  [28] .symtab           SYMTAB           0000000000000000  00003040
       0000000000000390  0000000000000018          29    19     8
  [29] .strtab           STRTAB           0000000000000000  000033d0
       00000000000001f4  0000000000000000           0     0     1
  [30] .shstrtab         STRTAB           0000000000000000  000035c4
       000000000000011a  0000000000000000           0     0     1
Key to Flags:
  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
  L (link order), O (extra OS processing required), G (group), T (TLS),
  C (compressed), x (unknown), o (OS specific), E (exclude),
  D (mbind), l (large), p (processor specific)
```

## `readelf -h` (ELF headers)

### `readelf -h main.o`

```
ELF Header:
  Magic:   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00
  Class:                             ELF64
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              REL (Relocatable file)
  Machine:                           Advanced Micro Devices X86-64
  Version:                           0x1
  Entry point address:               0x0
  Start of program headers:          0 (bytes into file)
  Start of section headers:          696 (bytes into file)
  Flags:                             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           0 (bytes)
  Number of program headers:         0
  Size of section headers:           64 (bytes)
  Number of section headers:         14
  Section header string table index: 13
```

### `readelf -h math_utils.o`

```
ELF Header:
  Magic:   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00
  Class:                             ELF64
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              REL (Relocatable file)
  Machine:                           Advanced Micro Devices X86-64
  Version:                           0x1
  Entry point address:               0x0
  Start of program headers:          0 (bytes into file)
  Start of section headers:          480 (bytes into file)
  Flags:                             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           0 (bytes)
  Number of program headers:         0
  Size of section headers:           64 (bytes)
  Number of section headers:         12
  Section header string table index: 11
```

### `readelf -h square_prog`

```
ELF Header:
  Magic:   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00
  Class:                             ELF64
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              DYN (Position-Independent Executable file)
  Machine:                           Advanced Micro Devices X86-64
  Version:                           0x1
  Entry point address:               0x1060
  Start of program headers:          64 (bytes into file)
  Start of section headers:          14048 (bytes into file)
  Flags:                             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           56 (bytes)
  Number of program headers:         13
  Size of section headers:           64 (bytes)
  Number of section headers:         31
  Section header string table index: 30
```

