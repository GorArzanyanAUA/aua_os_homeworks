// Use fork and execl to run a command with multiple arguments.

// Prepare a test.txt file with some text.
// Write a program that:
// Uses fork to create a child process.
// The child process should use execl to run the grep command to search for a specific word (e.g., "main") in a text file (e.g., test.txt).
// The parent process should print "Parent process completed".
// Expected Output: The lines in the file test.txt that contain the word "main" followed by "Parent process completed".


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
