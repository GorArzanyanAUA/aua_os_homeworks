// Assignment 2: File truncation
// Objective: Demonstrate writing, truncating, and verifying file size/content.
// Use the fixed filename "data.txt".
// Open/create it with O_WRONLY|O_CREAT|O_TRUNC (0644) and write the ASCII text: ABCDEFGHIJKLMNOPQRSTUVWXYZ (26 bytes). Close.
// Reopen with O_RDWR. Print the current size (by lseek(fd, 0, SEEK_END)).
// Truncate the file to the first 10 bytes using ftruncate.
// Print the new size (again via lseek).
// Rewind (lseek to 0) and read the remaining content, then print it to stdout.

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define FILENAME "data.txt"
#define NEW_SIZE 10

int main(){
    const char *text = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    ssize_t len = strlen(text);         // 26, the '\0' is not written to the file

    int fd = open(FILENAME, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1){
        perror("open for writing");
        return 1;
    }
    // write may write fewer bytes than asked, so keep writing until the whole text is out
    ssize_t written = 0;
    while (written < len){
        ssize_t w = write(fd, text + written, len - written);
        if (w == -1){
            perror("write");
            close(fd);
            return 1;
        }
        written += w;
    }
    if (close(fd) == -1){
        perror("close");
        return 1;
    }

    fd = open(FILENAME, O_RDWR);
    if (fd == -1){
        perror("open for reading/writing");
        return 1;
    }

    // seeking to the end returns the offset of the end, which is the file size
    off_t size = lseek(fd, 0, SEEK_END);
    if (size == -1){
        perror("lseek");
        close(fd);
        return 1;
    }
    printf("Size before truncation: %lld bytes\n", (long long)size);

    // ftruncate cuts the file to NEW_SIZE bytes; it does not move the file offset
    if (ftruncate(fd, NEW_SIZE) == -1){
        perror("ftruncate");
        close(fd);
        return 1;
    }

    size = lseek(fd, 0, SEEK_END);
    if (size == -1){
        perror("lseek");
        close(fd);
        return 1;
    }
    printf("Size after truncation: %lld bytes\n", (long long)size);

    // the offset is at the end now, so go back to the start before reading
    if (lseek(fd, 0, SEEK_SET) == -1){
        perror("lseek");
        close(fd);
        return 1;
    }

    char buf[64];
    ssize_t n;
    printf("Content after truncation: ");
    while ((n = read(fd, buf, sizeof(buf))) > 0)
        printf("%.*s", (int)n, buf);    // buf has no '\0', so print exactly n bytes
    printf("\n");
    if (n == -1){
        perror("read");
        close(fd);
        return 1;
    }

    if (close(fd) == -1){
        perror("close");
        return 1;
    }
    return 0;
}

