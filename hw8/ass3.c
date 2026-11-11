// Assignment 3: Reverse file reader
// Objective: Print a file's bytes in reverse order using lseek to move backward.
// Prompt the user to enter a path to an existing text file.
// Open it read-only. Determine file size with lseek(fd, 0, SEEK_END).
// Starting from the last byte, repeatedly: lseek to the target position, read 1 byte, write that byte to standard output.
// After finishing, write a newline.
// Handle empty files (print just a newline).

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(){
    char path[4096];     // longest allowed path

    printf("Enter the file path: ");
    if (scanf("%4095s", path) != 1){
        printf("Invalid file path\n");
        return 1;
    }

    int fd = open(path, O_RDONLY);
    if (fd == -1){
        perror("open");
        return 1;
    }

    off_t size = lseek(fd, 0, SEEK_END);
    if (size == -1){
        perror("lseek");
        close(fd);
        return 1;
    }

    // for an empty file size is 0, the loop does not run and only the newline is written
    char c;
    for (off_t pos = size - 1; pos >= 0; pos--){
        if (lseek(fd, pos, SEEK_SET) == -1){
            perror("lseek");
            close(fd);
            return 1;
        }
        ssize_t n = read(fd, &c, 1);
        if (n != 1){                    // -1 is an error, 0 means the file got shorter meanwhile
            if (n == -1) perror("read");
            else printf("read: unexpected end of file\n");
            close(fd);
            return 1;
        }
        if (write(STDOUT_FILENO, &c, 1) != 1){
            perror("write");
            close(fd);
            return 1;
        }
    }

    if (write(STDOUT_FILENO, "\n", 1) != 1){
        perror("write");
        close(fd);
        return 1;
    }

    if (close(fd) == -1){
        perror("close");
        return 1;
    }
    return 0;
}

