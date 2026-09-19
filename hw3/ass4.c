// Handle multiple processes and their exit statuses.
// Write a program that creates two child processes using fork().
// Each child process should return a different exit code using exit().
// The parent process should use waitpid() to wait for each child and print their exit status.
// Print whether each child exited normally or with an error.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


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
    if (ret1 == 0){ // child
        printf("First Child\n");
        exit(10);
    } else if (ret1 < 0) {
        perror("fork");
        return 1;
    }

    int ret2 = fork();
    if (ret2 == 0){ // child
        printf("Second Child\n");
        exit(20);
    } else if (ret2 < 0) {
        perror("fork");
        return 1;
    }

    int status1;
    int status2;


    if (waitpid(ret1, &status1, 0) > 0)
        report(ret1, status1);

    if (waitpid(ret2, &status2, 0) > 0)
        report(ret2, status2);

    return 0;
}

// "Normally" vs "with an error"
// -----------------------------
// These are two separate questions and need two different macros.
//   WIFEXITED(status)  -- did the child reach exit()/return at all, rather than
//                         being killed by a signal? That is the "normally" part.
//   WEXITSTATUS(status) -- the code it passed to exit(), only meaningful once
//                         WIFEXITED is true. Zero means success, anything else is
//                         the error the child is reporting.
// Both children here exit normally, but with codes 10 and 20, so both are normal
// terminations reporting an error. A child killed with SIGKILL would be the other
// case: no exit code at all, and WIFSIGNALED true instead.

