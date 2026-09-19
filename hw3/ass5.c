// Demonstrate zombie processes and prevent them from using wait().
// Write a program that creates a child process but does not use wait() in the parent.
// Use the  htop, top  or ps commands to observe the child process becoming a zombie.
// Modify the program to use wait() in the parent to prevent the child from becoming a zombie.
// Explain how wait() and waitpid() prevent zombie processes.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


// A zombie is a child that has already exited while its parent is still running and
// has not collected its status yet. So the child has to finish FIRST and the parent
// has to stay alive afterwards -- the opposite of making the child sleep.
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

// Same program with the one missing call added.
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

int main(){
    // make_zombie();
    no_zombie();
    return 0;
}

// How wait() and waitpid() prevent zombies
// ----------------------------------------
// When a process exits, the kernel frees its memory, its open files and almost
// everything else, but it cannot free the process table entry yet: that entry still
// holds the exit status, and the parent has not read it. A process in that state --
// finished, but still holding an entry so its status can be collected -- is a zombie.
// In ps it shows state Z and its name appears as <defunct>; htop shows it in red.
//
// wait() and waitpid() are what read that status. Once the parent has taken it, the
// kernel has no reason to keep the entry, so it removes it and the zombie disappears.
// Reaping is the only thing that clears a zombie -- a zombie is already dead, so it
// cannot be killed, and kill -9 on it does nothing.
//
// wait() blocks until any one child is reaped; waitpid() names a specific child.
// The difference does not matter for clearing the zombie -- both read the status, 
// which is what frees the entry.
//
// If the parent exits without ever reaping, the zombie is not stuck forever: an
// orphaned child is re-parented to init (PID 1), which reaps it. That is why the
// zombie in make_zombie() disappears the moment the parent exits, and why the
// observation has to happen during the sleep. The real problem is a long-running
// parent, like a server forking a worker per request: it never exits, so nothing
// reaps for it, and the entries pile up until the process table fills.
