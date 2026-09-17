// Understand how to pass arguments to programs executed with execl.

// Write a program that:
// Uses fork to create a child process.
// The child process should use execl to run the echo command with an argument (e.g., "Hello from the child process").
// The parent process should print "Parent process done" after the child process is created.
// Expected Output: The message from the echo command followed by the parent's message.

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