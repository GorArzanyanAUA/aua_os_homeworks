// Assignment 6: In-place overwrite using lseek
// Objective: Write numbers 1..10 as decimal text (one per line), then overwrite the 4th number with 100 using lseek.
// Fixed filename: "numbers.txt".
// Create/truncate and write the lines: "1\n" through "10\n". Close.
// Reopen with O_RDWR. You must compute the byte offset of the start of line 4 by scanning the file
// (read and count newlines), then use lseek to position there.
// Overwrite the 4th line's number with "100" and ensure the line still ends with '\n'. If the new text length
// differs (e.g., "4\n" -> "100\n"), you must shift the subsequent content correctly. Implement either:
// a) Read the remainder into memory, lseek back, write "100\n", then write the remainder; or
// b) Create a temporary file and reconstruct.
// Print the final file content to stdout at the end for verification.
//
// This solution uses option a).

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define FILENAME "numbers.txt"
#define TARGET_LINE 4
#define NEW_TEXT "100\n"

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
    // ---- step 1: create the file with "1\n" .. "10\n" ----
    int fd = open(FILENAME, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1){
        perror("open for writing");
        return 1;
    }
    for (int i = 1; i <= 10; i++){
        char num[16];
        int len = snprintf(num, sizeof(num), "%d\n", i);    // int -> text, write only sends bytes
        if (write_all(fd, num, len) == -1){
            perror("write");
            close(fd);
            return 1;
        }
    }
    if (close(fd) == -1){
        perror("close");
        return 1;
    }

    // ---- step 2: reopen and find where line 4 starts and ends by counting newlines ----
    fd = open(FILENAME, O_RDWR);
    if (fd == -1){
        perror("open for reading/writing");
        return 1;
    }

    // line 4 starts right after the 3rd '\n' and the next line starts right after the 4th '\n'
    off_t line_start = -1, next_line_start = -1;
    off_t pos = 0;                      // offset of buf[0] in the file
    int newlines = 0;
    char buf[64];
    ssize_t n;
    if (TARGET_LINE == 1) line_start = 0;
    while (next_line_start == -1 && (n = read(fd, buf, sizeof(buf))) > 0){
        for (ssize_t i = 0; i < n; i++){
            if (buf[i] != '\n') continue;
            newlines++;
            if (newlines == TARGET_LINE - 1) line_start = pos + i + 1;
            if (newlines == TARGET_LINE){
                next_line_start = pos + i + 1;
                break;
            }
        }
        pos += n;
    }
    if (n == -1){
        perror("read");
        close(fd);
        return 1;
    }
    if (next_line_start == -1){
        printf("The file has fewer than %d lines\n", TARGET_LINE);
        close(fd);
        return 1;
    }
    printf("Line %d starts at byte %lld\n", TARGET_LINE, (long long)line_start);

    // ---- step 3: read everything after line 4 into memory ----
    off_t size = lseek(fd, 0, SEEK_END);
    if (size == -1){
        perror("lseek");
        close(fd);
        return 1;
    }
    ssize_t rest_len = size - next_line_start;
    char *rest = malloc(rest_len > 0 ? rest_len : 1);
    if (rest == NULL){
        perror("malloc");
        close(fd);
        return 1;
    }
    if (lseek(fd, next_line_start, SEEK_SET) == -1){
        perror("lseek");
        free(rest);
        close(fd);
        return 1;
    }
    ssize_t got = 0;
    while (got < rest_len && (n = read(fd, rest + got, rest_len - got)) > 0)
        got += n;
    if (n == -1){
        perror("read");
        free(rest);
        close(fd);
        return 1;
    }

    // ---- step 4: go back to line 4, write the new line, then the saved rest after it ----
    // "4\n" is 2 bytes and "100\n" is 4, so writing "100\n" alone would overwrite "5\n";
    // that is why the rest was saved first and is written again right after the new text
    if (lseek(fd, line_start, SEEK_SET) == -1){
        perror("lseek");
        free(rest);
        close(fd);
        return 1;
    }
    if (write_all(fd, NEW_TEXT, strlen(NEW_TEXT)) == -1 || write_all(fd, rest, got) == -1){
        perror("write");
        free(rest);
        close(fd);
        return 1;
    }
    free(rest);
    rest = NULL;

    // if the new text were shorter than the old line, the old last bytes would remain at
    // the end, so cut the file at the current offset (here it only grows, so nothing is cut)
    off_t end = lseek(fd, 0, SEEK_CUR);
    if (end == -1 || ftruncate(fd, end) == -1){
        perror("lseek/ftruncate");
        close(fd);
        return 1;
    }

    // ---- step 5: print the final content ----
    if (lseek(fd, 0, SEEK_SET) == -1){
        perror("lseek");
        close(fd);
        return 1;
    }
    printf("Final content of %s:\n", FILENAME);
    while ((n = read(fd, buf, sizeof(buf))) > 0)
        printf("%.*s", (int)n, buf);    // buf has no '\0', so print exactly n bytes
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

