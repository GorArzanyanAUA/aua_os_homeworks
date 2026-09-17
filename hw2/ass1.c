// Understand the creation of a child process using fork and how to replace it with a new program using execl.
// Uses fork to create a child process.
// In the child process, use execl to run the ls command to list the contents of the current directory.
// The parent process should print "Parent process done" after the child process is created.
// Expected Output: The directory listing should be printed, followed by the parent's message.

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
