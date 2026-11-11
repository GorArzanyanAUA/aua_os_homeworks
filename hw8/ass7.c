// Assignment 7: Byte-wise file comparator
// Objective: Compare two files and report the first differing byte index.
// Prompt the user for the first file path, then the second file path.
// Open both read-only.
// Read both in fixed-size chunks and compare byte-by-byte.
// If a difference is found, print: "Files differ at byte N" (0-based index) and exit with non-zero code.
// If one file ends earlier but all preceding bytes matched, report the first differing byte as the end
// position of the shorter file.
// If no difference is found and sizes are equal, print: "Files are identical".
//
// Exit codes (same as the cmp command): 0 identical, 1 files differ, 2 error.

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define BUF_SIZE 4096   // usual memory page size and also usual filesystem block size

// read may return fewer bytes than asked even before EOF, so keep reading until the
// buffer is full or EOF; then both files give chunks of the same size until one ends
// returns the number of bytes read (less than len only at EOF), or -1 on error
ssize_t read_full(int fd, char *buf, ssize_t len){
    ssize_t got = 0;
    while (got < len){
        ssize_t n = read(fd, buf + got, len - got);
        if (n == -1)
            return -1;
        if (n == 0)                     // EOF
            break;
        got += n;
    }
    return got;
}

int main(){
    char path1[4096], path2[4096];      // longest allowed path

    printf("Enter the first file path: ");
    if (scanf("%4095s", path1) != 1){
        printf("Invalid file path\n");
        return 2;
    }
    printf("Enter the second file path: ");
    if (scanf("%4095s", path2) != 1){
        printf("Invalid file path\n");
        return 2;
    }

    int fd1 = open(path1, O_RDONLY);
    if (fd1 == -1){
        perror("open first file");
        return 2;
    }
    int fd2 = open(path2, O_RDONLY);
    if (fd2 == -1){
        perror("open second file");
        close(fd1);
        return 2;
    }

    char buf1[BUF_SIZE], buf2[BUF_SIZE];
    long long offset = 0;               // index of buf1[0] / buf2[0] in the files
    int result = 0;                     // 0 identical, 1 differ, 2 error

    while (1){
        ssize_t n1 = read_full(fd1, buf1, BUF_SIZE);
        ssize_t n2 = read_full(fd2, buf2, BUF_SIZE);
        if (n1 == -1 || n2 == -1){
            perror("read");
            result = 2;
            break;
        }

        // compare the bytes both chunks have
        ssize_t common = n1 < n2 ? n1 : n2;
        ssize_t i = 0;
        while (i < common && buf1[i] == buf2[i])
            i++;

        if (i < common){                // found a different byte
            printf("Files differ at byte %lld\n", offset + i);
            result = 1;
            break;
        }
        if (n1 != n2){                  // all common bytes match, but one file ended earlier
            printf("Files differ at byte %lld\n", offset + common);
            result = 1;
            break;
        }
        if (n1 == 0){                   // both ended at the same time with no difference
            printf("Files are identical\n");
            break;
        }
        offset += n1;
    }

    if (close(fd1) == -1){
        perror("close first file");
        result = 2;
    }
    if (close(fd2) == -1){
        perror("close second file");
        result = 2;
    }
    return result;
}

