// Learn how to use wait() and waitpid() for process synchronization.
// Modify the program from Assignment 1 to use wait() in the parent process to wait for the child process to complete.
// Add another child process using fork(), and use waitpid() in the parent to wait for a specific child process to finish first.
// Print the exit status of the child processes using the status code returned by wait() and waitpid().

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

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
