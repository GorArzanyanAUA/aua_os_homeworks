//  Understand process creation using fork().
// Task:
// Write a program that creates a child process using fork().
// The child process should print its PID and exit.
// The parent process should print its own PID without waiting for the child to complete.
// Use getpid() to print the PIDs of both the parent and child processes.

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