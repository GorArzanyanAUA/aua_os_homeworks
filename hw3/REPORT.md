# Homework 3

**Repository:** https://github.com/GorArzanyanAUA/aua_os_homeworks

**Author:** Gor Arzanyan


```
cd hw3
gcc ass1.c -o ../bin/ass1     # same for ass1 .. ass5
../bin/ass1
```


`ass3` and `ass5` are built to be run more than once: each contains several cases that
are selected by uncommenting one line, in the same style as `ass0.c` from Homework 2.

---

## Background — how a process reports its death


### The status word

`wait` and `waitpid` do not hand back the exit code directly. They fill in a **status
word** that packs together *how* the child died and *what* it reported, and the `W*`
macros take it apart:

| macro | question it answers |
|---|---|
| `WIFEXITED(status)` | did it reach `exit()`/`return`, rather than being killed? |
| `WEXITSTATUS(status)` | the code it passed to `exit()` — only valid if `WIFEXITED` |
| `WIFSIGNALED(status)` | was it killed by a signal instead? |
| `WTERMSIG(status)` | which signal — only valid if `WIFSIGNALED` |

Because the code is packed into the status word, **only the low 8 bits survive**.

---

## Assignment 1 — `fork()` without `wait()`

```c
#include <stdio.h>
#include <unistd.h>

int main(){
    int ret = fork();
    if (ret == 0){
        printf("Child pid : %d\n", getpid());
        return 0;
    } else if (ret > 0) {
        printf("Parent pid %d\n", getpid());
        return 0;
    } else {
        perror("fork");
    }
}
```

**Flow of execution.** `fork()` returns twice — `0` in the child, the child's PID in the
parent — so each branch runs in a different process. 

**Hierarchy.** Two processes, 164951 the parent and 164952 its only child.

**Output:**

```
Parent pid 164951
Child pid : 164952
```

**The order is not guaranteed.** 

---

## Assignment 2 — `wait()` and `waitpid()`

```c
int main(){
    printf("Parent pid %d\n", getpid());

    int ret = fork();
    if (ret == 0){
        printf("Child 1 pid : %d\n", getpid());
        return 42;
    } else if (ret < 0) {
        perror("fork");
        return 1;
    }

    int ret2 = fork();
    if (ret2 == 0){
        printf("Child 2 pid : %d\n", getpid());
        return 7;
    } else if (ret2 < 0) {
        perror("fork");
        return 1;
    }

    // waitpid() waits for one specific child
    int status2;
    int done2 = waitpid(ret2, &status2, 0);
    if (done2 > 0 && WIFEXITED(status2))
        printf("Child %d exited with code %d\n", done2, WEXITSTATUS(status2));

    // wait() takes whichever child is left, without naming it.
    int status1;
    int done1 = wait(&status1);
    if (done1 > 0 && WIFEXITED(status1))
        printf("Child %d exited with code %d\n", done1, WEXITSTATUS(status1));
    return 0;
}
```

**Flow of execution.** The parent forks twice. Each child prints and returns a distinct
code — 42 and 7 — so the two can be told apart by status alone. Neither child reaches the
second `fork`, because each returns before it.

**Hierarchy.** Three processes: parent 164954 with two siblings 164955 and 164956
underneath. The children never coexist with each other's code paths.

**Output:**

```
Parent pid 164954
Child 1 pid : 164955
Child 2 pid : 164956
Child 164956 exited with code 7
Child 164955 exited with code 42
```

**`waitpid` names its child; `wait` does not.** The reports come out in the order
*child 2, then child 1* — the reverse of creation. That is deliberate, and it is the
whole point of the assignment: `waitpid(ret2, ...)` blocks specifically for child 2 even
if child 1 finished first, so child 2's status is always collected first. The `wait()`
that follows simply takes whoever is left.


---

## Assignment 3 — `atexit()`

```c
void func1 (){ printf("First function call\n"); }
void func2 (){ printf("Second function call\n"); }

void func_exit (){
    printf("Leaving from func_exit()\n");
    exit(3);
}

int main() {
    printf("Program started\n");

    if (atexit(func1) != 0) { perror("atexit"); }
    if (atexit(func2) != 0) { perror("atexit"); }

    // 1) exit(1);        2) func_exit();        3) _exit(0);
    // 4) fall through:
    printf("Program finished\n");
    return 0;
}
```

**Flow of execution.** `atexit` stores a pointer to a `void(void)`. The list is walked later, when
the process terminates through `exit`.

**Handlers run in reverse order of registration (LIFO).** `func1` is registered first, so
it runs *last*. This is not an accident of implementation: it is what makes `atexit`
usable for cleanup, because resources are released in the reverse of the order they were
acquired, so a handler registered later can still rely on anything set up before it.

**The four exit paths, and what each prints:**

| # | how the process leaves | output | status |
|---|---|---|---|
| 1 | `exit(1)` in the middle of `main` | `Program started` / `Second` / `First` | 1 |
| 2 | `exit(3)` inside `func_exit()` | `Program started` / `Leaving from func_exit()` / `Second` / `First` | 3 |
| 3 | `_exit(0)` | `Program started` only | 0 |
| 4 | `return 0` from `main` | `Program started` / `Program finished` / `Second` / `First` | 0 |
| 5 | killed by a signal — `raise(SIGTERM)` | `Program started` only | 143 |
| 6 | `abort()` (raises `SIGABRT`) | `Program started` only | 134 |

Case 4, in full:

```
Program started
Program finished
Second function call
First function call
```

**Cases 1, 2 and 4 are the same mechanism.** It does not matter whether `exit` is called
in the middle of `main`, from a function nested inside it, or reached by falling off the
end of `main` — all three go through the library's exit sequence, so all three run both
handlers, in the same LIFO order. Case 2 is the interesting one for real code: `exit`
terminates the process from wherever it is reached, so the cleanup happens even for an
exit buried deep inside a call chain that the author of `main` never sees.

**Case 3 is different, in two ways.** `_exit` is the raw kernel call and skips the exit
sequence entirely, so neither handler runs. It *also* loses buffered output. Run on a
terminal it prints `Program started`, but piped it prints nothing at all:

```
$ ./ass3            # terminal, line-buffered
Program started

$ ./ass3 | cat      # pipe, block-buffered
                    # (nothing)
```

Both effects have the same cause — `_exit` does not flush stdio — and the difference
between the two runs is only *when* the buffer would have been flushed. A terminal is
line-buffered, so the newline already pushed the text out; a pipe is block-buffered, so
the line was still sitting in the buffer when the process vanished.

**Cases 5 and 6 — a signal never enters `exit` at all.** Unlike `_exit`, where the
program still chooses to leave and just takes the short route, a signalled process is
torn down by the kernel with no cooperation from the program, so the handler list is
never walked. `abort()` is the same thing in disguise — it raises `SIGABRT`. Both
statuses follow the shell's `128 + signal` convention, 143 for `SIGTERM` and 134 for
`SIGABRT`, so a status above 128 is itself the sign that the process was signalled
rather than having returned a code of its own.

This is the gap that matters in practice, since `SIGINT` from Ctrl-C and `SIGTERM` from
`kill` are exactly when cleanup is wanted. Catching the signal and calling `exit` from
the handler closes it — both handlers run again — which shows the real rule: **the
handler list is walked by `exit`, and only by `exit`**. `SIGKILL` is the exception, as it
cannot be caught.

**Why `atexit` is useful in practice.** It puts cleanup in one place instead of repeating
it before every `exit` in the program. A real program leaves through many paths — a usage
error, a failed allocation, an `exit` inside a library — and each of them has to flush and
close log files, delete temporary files, remove a PID or lock file, unlink a socket, or
restore a terminal that was put into raw mode. Registering the handler once covers all of
them.

---

## Assignment 4 — Two children and their exit statuses

```c
void report (int pid, int status){
    if (WIFEXITED(status)) {
        int code = WEXITSTATUS(status);
        if (code == 0)
            printf("Child %d exited normally, status %d\n", pid, code);
        else
            printf("Child %d exited normally but reported an error, status %d\n", pid, code);
    } else if (WIFSIGNALED(status)) {
        printf("Child %d was killed by signal %d\n", pid, WTERMSIG(status));
    } else {
        printf("Child %d stopped without exiting\n", pid);
    }
}

int main() {
    int ret1 = fork();
    if (ret1 == 0){ printf("First Child\n");  exit(10); }
    else if (ret1 < 0) { perror("fork"); return 1; }

    int ret2 = fork();
    if (ret2 == 0){ printf("Second Child\n"); exit(20); }
    else if (ret2 < 0) { perror("fork"); return 1; }

    int status1, status2;
    if (waitpid(ret1, &status1, 0) > 0) report(ret1, status1);
    if (waitpid(ret2, &status2, 0) > 0) report(ret2, status2);
    return 0;
}
```

**Flow of execution.** Two children exit with 10 and 20. The parent `waitpid`s for each by
name and decodes the status word. Decoding is factored into `report()` rather than written
twice.


**Output:**

```
First Child
Second Child
Child 171054 exited normally but reported an error, status 10
Child 171055 exited normally but reported an error, status 20
```

**"Normally" and "with an error" are two separate questions.** The assignment asks the
program to say which one happened, and no single macro answers it — they need two:

- `WIFEXITED` asks whether the child reached `exit()` at all, rather than being killed by
  a signal. That is the *normally* part.
- `WEXITSTATUS` asks what it reported once it got there. Zero is success; anything else is
  an error the child is reporting about its own work.

These two children land in the combination that is easy to misread. They exit **normally**
— no signal, no crash — but with **non-zero codes**, so they are normal terminations
carrying an error. The wording in the output says exactly that rather than collapsing the
two questions into one.

**The abnormal case, for comparison.** Changing child 1 to `exit(0)` and child 2 to
`raise(SIGKILL)` exercises the other two branches:

```
Child 111828 exited normally, status 0
Child 111829 was killed by signal 9
```

A child killed by a signal has **no exit code at all** — `WEXITSTATUS` would be
meaningless — which is why `WIFEXITED` has to be checked before it is read.

**Why not just compare `status` to 0.** The status word is packed, not a plain code. It
happens to compare equal to zero for a child that exited with 0, which makes the mistake
easy to miss, but for any other case the number is not the exit code.

---

## Assignment 5 — Zombie processes

A **zombie** is a process that has already terminated but whose parent has not yet
collected its status. The kernel has freed its memory, its open files and almost
everything else, but it cannot free the **process table entry** — that entry is still
holding the exit status, and nobody has read it yet.

For that state to exist, the child has to finish **first** and the parent has to stay
alive **afterwards**. The program is built around exactly that ordering, with both
versions in one file and selected from `main`:

```c
int main(){
    // make_zombie();
    no_zombie();
    return 0;
}
```

### 5a — `make_zombie()`: no `wait()` in the parent

```c
void make_zombie (){
    int ret = fork();
    if (ret == 0) { // child: exit at once, and become a zombie
        printf("Child %d exiting immediately\n", getpid());
        exit(0);
    } else if (ret < 0) {
        perror("fork");
        return;
    }

    printf("Parent %d will not call wait() for %d seconds\n", getpid(), 30);

    // No wait() here, so the kernel keeps the child's entry alive as a zombie.
    sleep(30);

    printf("Parent %d exiting\n", getpid());
}
```

**Flow of execution.** The child prints and exits immediately. The parent never calls
`wait`, and sleeps for 30 seconds — long enough to inspect the system while the child is
dead but uncollected.



**Confirmed with `ps`,** which reports the same thing and shows the name as `<defunct>`:

```
$ ps -o pid,ppid,state,comm,rss --ppid 168319
    PID    PPID S COMMAND           RSS
 168321  168319 Z mz                  0
```


**The zombie disappears when the parent exits.** Once the 30 seconds elapse:

```
$ ps -o pid,ppid,state,comm --ppid 168319
(none left)
```

An orphaned child is re-parented to `init` (PID 1), which reaps it immediately.

### 5b — `no_zombie()`: the parent calls `wait()`

```c
void no_zombie (){
    int ret = fork();
    if (ret == 0) { // child
        printf("Child %d exiting immediately\n", getpid());
        exit(0);
    } else if (ret < 0) {
        perror("fork");
        return;
    }

    // wait() collects the status, which lets the kernel drop the entry.
    int status;
    int done = wait(&status);
    sleep(10);
    if (done > 0 && WIFEXITED(status))
        printf("Parent %d reaped child %d, status %d\n", getpid(), done, WEXITSTATUS(status));

    printf("Parent %d will now idle for %d seconds\n", getpid(), 30);
    printf("Look for child %d now:  ps -o pid,ppid,state,comm -p %d\n", ret, ret);

    sleep(30);

    printf("Parent %d exiting\n", getpid());
}
```

**Flow of execution.** Identical to 5a apart from the `wait(&status)`. The `sleep` after it
keeps the parent alive with the same timing as before, so the two versions can be compared
under identical conditions — the only difference being whether the status was collected.

**Output:**

```
Child 167023 exiting immediately
Parent 167021 reaped child 167023, status 0
Parent 167021 will now idle for 30 seconds
Look for child 167023 now:  ps -o pid,ppid,state,comm -p 167023
Parent 167021 exiting
```

**Observed in `ps`:** no child at all, at any point during the parent's sleep.

```
$ ps -o pid,ppid,state,comm,rss --ppid 167021
(no child processes remain)
```


### How `wait()` and `waitpid()` prevent zombies

The entry survives after death for exactly one reason: it is holding the exit status, and
the parent has not read it. `wait` and `waitpid` are the calls that read it. Once the
parent has taken the status, the kernel has no reason to keep the entry, so it removes it —
and the zombie is gone. Reaping is the *only* thing that clears a zombie, since the process
is already dead and cannot be signalled.

The two calls differ in *which* child they collect, not in what collecting does:

| | `wait(&status)` | `waitpid(pid, &status, 0)` |
|---|---|---|
| which child | whichever finishes first | the one named by `pid` |
| blocks? | yes | yes, or poll with `WNOHANG` |
| clears the zombie | yes | yes |

---
