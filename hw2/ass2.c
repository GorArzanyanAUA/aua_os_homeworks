// Work with multiple child processes created using fork and run different commands using execl.

// Write a program that:
// Creates two child processes using fork.
// The first child process should use execl to run the ls command.
// The second child process should use execl to run the date command.
// The parent process should print "Parent process done" after creating both child processes.
// Expected Output: The output of the ls command followed by the output of the date command, and finally the parent's message.

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
    else if (ret1 == 0) {                       // first child : ls
        execl("/usr/bin/ls", "ls", NULL);
        perror("execl");                   // only reached if execl failed
    }  else {
        wait(NULL);                            // ls must finish before date starts

        int ret2 = fork();
        if (ret2 == -1) {
            perror("fork");
        }
        else if (ret2 == 0) {                       // second child : date
            execl("/usr/bin/date", "date", NULL);
            perror("execl");
        } else {
            wait(NULL);                            // date must finish before our message
            printf("Parent process done\n");       // parent, after both children
        }

    }

}
