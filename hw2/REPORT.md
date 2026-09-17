# Homework 2

**Repository:** https://github.com/GorArzanyanAUA/aua_os_homeworks

**Author:** Gor Arzanyan


```
cd hw2
gcc ass0.c -o ../bin/ass0     # same for ass1 .. ass4
../bin/ass0
```

`ass4` must be run from inside `hw2/`, because it greps `test.txt` by a relative path
and `exec` inherits the caller's working directory.

---

## Background — `fork` and `execl`

Actually, I found out thatNeither `fork` nor `execl` are system calls itself. Both are library functions provided
by glibc, and the trap into the kernel happens inside them.
### Return values used throughout

| call | returns in parent | returns in child | on failure |
|---|---|---|---|
| `fork()` | child's PID (`> 0`) | `0` | `-1`, `errno` set |
| `execl()` | *does not return* | *does not return* | `-1`, `errno` set |
| `wait(NULL)` | PID of a reaped child | — | `-1` when no children remain (`ECHILD`) |

---

## Assignment 0 — Multiple `fork`s and the resulting process tree

The assignment asks what the process tree looks like when several `fork` calls run one
after another. Rather than write a single program, I tried **three different
configurations of the same three `fork()` calls**, changing only *which* process is
allowed to run the next fork:

| configuration | which process performs the next fork |
|---|---|
| `plain_forks()` | every process — no branching on the return value at all |
| `one_parent_multiple_childs()` | only the original process |
| `grandpa_son_grandson_ggrandson()` | only the child that was just created |

All three live in `ass0.c` as separate functions and are selected from `main()`, one at a
time:

```c
int main(){
        // one_parent_multiple_childs();
        // grandpa_son_grandson_ggrandson();
        plain_forks();
        return 0; 
}
```

The source and the resulting tree for each configuration follow.

### 0a — `plain_forks()`: three unguarded forks

```c
void plain_forks (){

        //  Nobody branches on the return value, so EVERY process runs every
        //  fork that is left : the population doubles 3 times, 1 -> 2 -> 4 -> 8.
        //  A process born at fork n only runs the forks after n, so the tree is
        //  lopsided : the original ends up with 3 children, its first child with
        //  2, the next with 1, and the last four with none.
        printf("Start : pid %d, parent %d\n", getpid(), getppid());

        int ret1 = fork();           // 1 -> 2 processes
        if (ret1 == -1) {
                perror("fork");
        }
        int ret2 = fork();           // 2 -> 4 processes
        if (ret2 == -1) {
                perror("fork");
        }
        int ret3 = fork();           // 4 -> 8 processes
        if (ret3 == -1) {
                perror("fork");
        }

        printf("Alive : pid %d, parent %d\n", getpid(), getppid());

        while (wait(NULL) > 0);      // every process reaps whatever it forked
}
```

**Flow of execution.** Nothing branches on the return value, so *every* process that
exists runs *every* `fork()` that is still ahead of it. The population doubles three
times: 1 → 2 → 4 → 8.


**Hierarchy.** The eight processes are *not* eight siblings. A process born at fork *n*
only executes the forks after *n*, so the later a process is born the fewer children it
gets. Actual run (letters added in birth order, P0 is the original):

```
 16525 (P0) --+-- 16526 (A) --+-- 16529 (C) --- 16532 (G)
              |               |
              |               +-- 16531 (E)
              |
              +-- 16527 (B) --- 16530 (F)
              |
              +-- 16528 (D)
```

```
Start : pid 16525, parent 16523
Alive : pid 16525, parent 16523
Alive : pid 16528, parent 16525
Alive : pid 16527, parent 16525
Alive : pid 16526, parent 16525
Alive : pid 16530, parent 16527
Alive : pid 16531, parent 16526
Alive : pid 16529, parent 16526
Alive : pid 16532, parent 16529
```

Children per process: P0 has 3, A has 2, B and C have 1 each, D E F G have none —
7 children for 8 processes, as every tree of 8 nodes must have.


**Ordering.** The interleaving of the `Alive :` lines differs between runs — after a
`fork` both processes are runnable and the scheduler decides who prints first.

### 0b — `one_parent_multiple_childs()`: three forks from the same process

```c
void one_parent_multiple_childs () {

        //  time ------------------------------------------------>
        //
        //                 f1        f2        f3
        //  parent   ------+---------+---------+---[wait]--->
        //                 |         |         |
        //  child 3        |         |         \----------->
        //  child 2        |         \-------------------->
        //  child 1        \--------------------------->
        //
        //  All 3 forks run in the SAME process, so the 3 children are
        //  siblings : 4 processes, tree depth 1, one parent with 3 kids.
        printf("Process pid : %d\n", getpid());
        int ret1 = fork();
        if (ret1 > 0) {                       // parent
                int ret2 = fork();
                if (ret2 > 0) {              // parent
                        int ret3 = fork();
                        if (ret3 == 0) {
                                printf("Third child pid : %d, parent : %d\n", getpid(), getppid());
                        } else if (ret3 == -1) {
                                perror("fork");
                                while (wait(NULL) > 0);   // still reap the first two
                        } else {             // original process : has all 3 children
                                while (wait(NULL) > 0);   // reap them all
                        }
                } else if (ret2 == 0) {
                        printf("Second child pid : %d, parent : %d\n", getpid(), getppid());
                } else {
                        perror("fork");
                        while (wait(NULL) > 0);           // still reap the first child
                }
        } else if (ret1 == 0){               // child 
                printf("First child pid : %d, parent : %d\n", getpid(), getppid());
        } else {                             // error : no children to reap
                perror("fork");
        }
}
```

**Flow of execution.** Each fork is guarded so that only the branch with `ret > 0` — the
**parent** — continues forking. Every child takes its `ret == 0` branch, prints, and
returns without forking again. All three forks therefore happen in one single process.


**Hierarchy.** 4 processes, tree depth 1 — one parent with three siblings underneath:

```
Process pid : 16535
First child pid : 16536, parent : 16535
Second child pid : 16537, parent : 16535
Third child pid : 16538, parent : 16535
```

All three children report the same `getppid()`, which is what proves they are siblings
rather than a chain. This only holds because of the `wait` loop: without it the parent
would exit first and each child's `getppid()` would report the reaping `init`/`systemd`
process instead of the real parent.


### 0c — `grandpa_son_grandson_ggrandson()`: three forks down a chain

```c
void grandpa_son_grandson_ggrandson (){

        //  time ------------------------------------------------>
        //
        //                 f1        f2        f3
        //  grandpa  ------+----------------------------->
        //                 |
        //  son            \---------+--------------------->
        //                           |
        //  grandson                 \---------+----------->
        //                                     |
        //  ggrandson                          \----------->
        //
        //  Each fork runs in the CHILD made by the previous one, so the
        //  processes form a line : 4 processes, tree depth 3, every
        //  process has exactly 1 child.
        printf("Grandpa pid : %d\n", getpid());
        int ret1 = fork();
        if (ret1 == 0) {                       // son
                printf("Son pid : %d, parent : %d\n", getpid(), getppid());
                int ret2 = fork();
                if (ret2 == 0) {               // grandson
                        printf("Grandson pid : %d, parent : %d\n", getpid(), getppid());
                        int ret3 = fork();
                        if (ret3 == 0) {       // great-grandson
                                printf("Great-grandson pid : %d, parent : %d\n", getpid(), getppid());
                        } else if (ret3 == -1) {
                                perror("fork");
                        } else {               // grandson waits for great-grandson
                                wait(NULL);
                        }
                } else if (ret2 == -1) {
                        perror("fork");
                } else {                       // son waits for grandson
                        wait(NULL);
                }
        } else if (ret1 == -1){                // error
                perror("fork");
        } else {                               // grandpa waits for son
                wait(NULL);
        }
}
```

**Flow of execution.** The mirror image of 0b: the nested forks sit in the `ret == 0`
branch, so each new fork is performed by the **child** just created. Each parent takes
its `else` branch and does nothing but `wait`.

**Hierarchy.** 4 processes, tree depth 3, every process having exactly one child:

```
Grandpa pid : 16541
Son pid : 16542, parent : 16541
Grandson pid : 16543, parent : 16542
Great-grandson pid : 16544, parent : 16543
```

---

## Assignment 1 — `fork` + `execl` (run `ls`)

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(){
    int ret = fork();
    if (ret == 0) {                 // child : becomes ls, never returns on success
        execl("/usr/bin/ls", "ls", NULL);
        perror("execl");            // only reached if execl failed
    } else if (ret > 0) {           // parent
        wait(NULL);                 // let the listing finish before we print
        printf("Parent process done\n");
    } else {
        perror("fork");
        return 1;
    }
    return 0;
}
```

**Flow of execution.** `fork()` produces two processes running identical code. The child
sees `ret == 0` and calls `execl`, which **replaces its entire program image** — code,
data, heap and stack are discarded and `/usr/bin/ls` is loaded in their place. The
process keeps its PID, its parent, and its open file descriptors (which is why `ls`
writes to the same terminal), but nothing of the original program survives. The parent
sees `ret > 0`, blocks in `wait`, and prints once the child has terminated.

**Hierarchy.** Two processes. 16729 is the parent, 16730 its only child — and after the
`execve`, that child *is* `ls`. The parent/child relationship is unaffected by `exec`;
only the program being run changes.

**Output:**

```
ass0.c
ass1.c
ass2.c
ass3.c
ass4.c
REPORT.md
test.txt
Parent process done
```

**Why `wait` is required.** The assignment specifies the listing *followed by* the
parent's message. Without `wait(NULL)` the two processes race and the parent frequently
prints first. `wait` makes the stated order actually hold rather than merely likely.

---

## Assignment 2 — Two children running `ls` and `date`

```c
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main() {
    int ret1 = fork();
    if (ret1 == -1) {
        perror("fork");
    }
    else if (ret1 == 0) {                      // first child : ls
        execl("/usr/bin/ls", "ls", NULL);
        perror("execl");                       // only reached if execl failed
    }  else {
        wait(NULL);                            // ls must finish before date starts

        int ret2 = fork();
        if (ret2 == -1) {
            perror("fork");
        }
        else if (ret2 == 0) {                  // second child : date
            execl("/usr/bin/date", "date", NULL);
            perror("execl");
        } else {
            wait(NULL);                        // date must finish before our message
            printf("Parent process done\n");   // parent, after both children
        }
    }
}
```

**Flow of execution.** The parent forks the first child, which becomes `ls`. The parent
then **blocks in `wait` until `ls` has terminated**, and only afterwards forks the second
child, which becomes `date`. A second `wait` then precedes the parent's own message.

**The ordering problem.** The assignment requires `ls`, then `date`, then the message.
Forking both children up front and calling `wait` twice at the end would guarantee only
that the parent's message comes last — `ls` and `date` would run concurrently and their
output could interleave in either order.


**Hierarchy.** Three processes over the program's lifetime, but never more than two at
once. 16738 is the parent of both 16739 (`ls`) and 16740 (`date`); the two children are
siblings and never coexist.

**Output:**

```
ass0.c
ass1.c
ass2.c
ass3.c
ass4.c
REPORT.md
test.txt
Thu 17 Sep 2026 09:08:42 PM +04
Parent process done
```

---

## Assignment 3 — Passing an argument to `execl` (run `echo`)

```c
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
    int ret = fork();
    if (ret == 0){
        execl("/usr/bin/echo", "echo", "Hello from the child process", NULL);
        perror("execl");
    } else if (ret > 0) {
        wait(NULL);
        printf("Parent process done\n");
    } else {
        perror("fork");
    }
}
```

**Flow of execution.** Same two-process structure as Assignment 1; what changes is the
argument list handed to the new program.

**Hierarchy.** Two processes: parent 24558, child 24559 which becomes `echo`.

**Output:**

```
Hello from the child process
Parent process done
```

---

## Assignment 4 — `execl` with multiple arguments (run `grep`)

```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
    int ret = fork();
    if (ret == 0){                         // child
        execl("/usr/bin/grep", "grep", "main", "test.txt", NULL);
        perror("execl");                   // only reached if execl failed
    } else if (ret > 0) {                  // parent
        wait(NULL);                        // print only when child is done
        printf("Parent process completed\n");
    } else {
        perror("fork");
    }
    return 0;
}
```

`test.txt`:

```
some
main 1
some 
test
x
some
main 2
```

**Flow of execution.** Identical structure to Assignment 3, with a two-argument command.

**Hierarchy.** Two processes: parent 24567, child 24568 which becomes `grep`.

**Output:**

```
main 1
main 2
Parent process completed
```

**Working directory is inherited.** `exec` replaces the program image but keeps the
process's working directory, so `"test.txt"` is resolved relative to wherever the *parent*
was started. Running `./bin/ass4` from the repository root gives:

```
grep: test.txt: No such file or directory
Parent process completed
```

The `execl` succeeded here — the error came from `grep` itself failing to open the file,
which is a different failure from `execl` being unable to load the program. The program
must be run from `hw2/`, where `test.txt` lives.

---
