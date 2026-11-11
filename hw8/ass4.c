// Assignment 4: Append log entries with PID and final offset
// Objective: Append a user-typed line to a log file in O_APPEND mode and show the final file offset.
// Fixed filename: "log.txt".
// Open with O_WRONLY|O_CREAT|O_APPEND (0644).
// Read up to one line from standard input using read(0, ...). Do not use fgets/gets.
// Prepend "PID=<pid>: " to the line (getpid()) and write the result followed by a newline if not present.
// After writing, obtain and print the new file position by calling lseek(fd, 0, SEEK_CUR).
// Explain in a comment why SEEK_CUR still returns a growing offset even with O_APPEND.

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define FILENAME "log.txt"
#define LINE_SIZE 4096

int main(){
    int fd = open(FILENAME, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1){
        perror("open");
        return 1;
    }

    printf("Enter a log line: ");
    fflush(stdout);

    // read one byte at a time so we stop exactly at the end of the line;
    // a bigger read could take several lines at once when the input comes from a pipe or file
    char line[LINE_SIZE];
    int len = 0;
    ssize_t n;
    while (len < LINE_SIZE - 1 && (n = read(STDIN_FILENO, &line[len], 1)) == 1){
        len++;
        if (line[len - 1] == '\n')
            break;
    }
    if (n == -1){
        perror("read");
        close(fd);
        return 1;
    }
    if (len == 0){                      // EOF before anything was typed (e.g. Ctrl+D)
        printf("No input\n");
        close(fd);
        return 1;
    }
    if (line[len - 1] != '\n')          // if ended because of length
        line[len++] = '\n';

    // build the whole entry first and write it in one go, so the prefix and the line
    // can not be separated by another process appending to the same file in between
    // in case of two separate write calls this may happen.
    char entry[LINE_SIZE + 32];
    int prefix_len = snprintf(entry, sizeof(entry), "PID=%d: ", getpid());
    memcpy(entry + prefix_len, line, len);
    ssize_t entry_len = prefix_len + len;

    // write may write fewer bytes than asked, so keep writing until the whole entry is out
    ssize_t written = 0;
    while (written < entry_len){
        ssize_t w = write(fd, entry + written, entry_len - written);
        if (w == -1){
            perror("write");
            close(fd);
            return 1;
        }
        written += w;
    }

    // Why SEEK_CUR keeps growing even with O_APPEND:
    // O_APPEND does not disable the file offset. Before every write the kernel moves the
    // offset to the current end of the file, writes the data there, and then advances the
    // offset by the number of bytes written, just like a normal write. So right after our
    // write the offset points just past our entry, which is the new end of the file.
    // Every run adds an entry, so the file gets longer and the printed offset grows each time.
    // (If another process appended between our write and this lseek, the file could be
    // longer than this offset; SEEK_CUR is our own position, not the live file size.)
    off_t offset = lseek(fd, 0, SEEK_CUR);
    if (offset == -1){
        perror("lseek");
        close(fd);
        return 1;
    }
    printf("Wrote %zd bytes, new file offset: %lld\n", entry_len, (long long)offset);

    if (close(fd) == -1){
        perror("close");
        return 1;
    }
    return 0;
}

