// Assignment 1: Copy file contents
// Objective: Copy the full contents of one file to another using read/write loops.
// At the start, prompt the user (via standard input) to type the source file path, then the destination file path.
// Open the source for reading (O_RDONLY). Open/create the destination with O_WRONLY|O_CREAT|O_TRUNC and mode 0644.
// Repeatedly read from source and write to destination until EOF (read returns 0).
// Count and print the total number of bytes copied.
// Print an error if any system call fails.

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define BUF_SIZE 4096   // usual memory page size and also usual filesystem block size. 

int main(){
    char src_path[4096], dst_path[4096]; // maximum allowed path length

    printf("Enter the source file path: ");
    if (scanf("%4095s", src_path) != 1){
        printf("Invalid file path\n");
        return 1;
    }
    printf("Enter the destination file path: ");
    if (scanf("%4095s", dst_path) != 1){
        printf("Invalid file path\n");
        return 1;
    }

    int src = open(src_path, O_RDONLY);
    if (src == -1){
        perror("open source");
        return 1;
    }

    // 0644: owner can read/write, group and others can only read (umask may remove more bits)
    int dst = open(dst_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dst == -1){
        perror("open destination");
        close(src);
        return 1;
    }

    char buf[BUF_SIZE];
    long total = 0;
    ssize_t n;
    while ((n = read(src, buf, BUF_SIZE)) > 0){
        // write may write fewer bytes than asked, so keep writing until the whole chunk is out
        ssize_t written = 0;
        while (written < n){
            ssize_t w = write(dst, buf + written, n - written);
            if (w == -1){
                perror("write");
                close(src);
                close(dst);
                return 1;
            }
            written += w;
        }
        total += written;
    }
    if (n == -1){                       // loop ended because of an error, not EOF
        perror("read");
        close(src);
        close(dst);
        return 1;
    }

    if (close(src) == -1){
        perror("close source");
        close(dst);
        return 1;
    }
    if (close(dst) == -1){              // a delayed write error can show up here
        perror("close destination");
        return 1;
    }

    printf("Copied %ld bytes from %s to %s\n", total, src_path, dst_path);
    return 0;
}

