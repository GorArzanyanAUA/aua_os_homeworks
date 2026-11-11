// Assignment 5: Sparse file creator
// Objective: Create a sparse file by seeking beyond the end and writing at a far offset.
// Fixed filename: "sparse.bin".
// Open/create with O_WRONLY|O_CREAT|O_TRUNC (0644).
// Write the bytes "START".
// Use lseek to skip forward by 1 MiB from the current position (1024*1024). Do not write zeros manually.
// Write the bytes "END". Close the file.
// Reopen read-only, seek to end, and print the apparent file size (should be >= 1,048,581 bytes).
// Add a code comment explaining why disk usage (du) is much smaller than the apparent size.

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define FILENAME "sparse.bin"
#define SKIP (1024 * 1024)

// write may write fewer bytes than asked, so keep writing until all len bytes are out
int write_all(int fd, const char *buf, ssize_t len){
    ssize_t written = 0;
    while (written < len){
        ssize_t w = write(fd, buf + written, len - written);
        if (w == -1)
            return -1;
        written += w;
    }
    return 0;
}

int main(){
    int fd = open(FILENAME, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1){
        perror("open for writing");
        return 1;
    }

    if (write_all(fd, "START", 5) == -1){
        perror("write START");
        close(fd);
        return 1;
    }

    // move the offset 1 MiB past the end; lseek alone does not change the file size
    if (lseek(fd, SKIP, SEEK_CUR) == -1){
        perror("lseek");
        close(fd);
        return 1;
    }

    // writing at the far offset extends the file; the 1 MiB in between becomes a "hole"
    if (write_all(fd, "END", 3) == -1){
        perror("write END");
        close(fd);
        return 1;
    }

    if (close(fd) == -1){
        perror("close");
        return 1;
    }

    fd = open(FILENAME, O_RDONLY);
    if (fd == -1){
        perror("open for reading");
        return 1;
    }

    off_t size = lseek(fd, 0, SEEK_END);
    if (size == -1){
        perror("lseek");
        close(fd);
        return 1;
    }
    // 5 ("START") + 1048576 (hole) + 3 ("END") = 1048584
    printf("Apparent file size: %lld bytes\n", (long long)size);

    // Why du shows much less than the apparent size:
    // The apparent size (ls -l, lseek SEEK_END) is just the offset of the last byte + 1.
    // Disk usage (du) counts the blocks that are actually allocated on disk.
    // We never wrote the 1 MiB between "START" and "END"; we only moved the offset over it.
    // The filesystem does not allocate blocks for a range that was never written, it just
    // records that the range is a hole. Reading a hole returns zeros, but those zeros are not
    // stored anywhere. Only the block(s) holding "START" and "END" use space, so du reports
    // a few KiB (e.g. 8K = two 4 KiB blocks) instead of ~1 MiB.
    // Compare: ls -l sparse.bin   vs   du -h sparse.bin

    if (close(fd) == -1){
        perror("close");
        return 1;
    }
    return 0;
}

