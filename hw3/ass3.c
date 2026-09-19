// Learn how to use the atexit() function for process cleanup.
// Write a program that registers two functions using atexit().
// The functions should print messages when the program terminates.
// Test the program by calling exit() in different locations and observing the order of function calls.
// Explain how atexit() can be useful in real-world applications.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void func1 (){
    printf("First function call\n");
}

void func2 (){
    printf("Second function call\n");
}


void func_exit (){
    printf("Leaving from func_exit()\n");
    exit(3);
}

int main() {
    printf("Program started\n");

    // atexit() stores a pointer to a void(void) function and returns 0 on success.
    // func1 is registered first, so it runs last.
    if (atexit(func1) != 0) {
        perror("atexit");
    }

    if (atexit(func2) != 0) {
        perror("atexit");
    }
    sleep(5);
    // Four ways to leave the process. Uncomment one at a time and compare the output.

    // 1) exit() in the middle of main
    // exit(1);

    // 2) exit() from inside another function
    // func_exit();

    // 3) _exit() skips the handlers entirely -- it is the raw system call and
    //    does not go through the C library's exit sequence.
    // _exit(0);

    // 4) falling off the end of main: returning from main is equivalent to exit()
    printf("Program finished\n");
    return 0;
}

// Order of the calls
// ------------------
// In cases 1, 2 and 4 the output ends with
//     Second function call
//     First function call
// The handlers run in reverse order of registration.
//
// Case 3 prints neither message, because _exit() ends the process without running
// the handlers. 
// It also loses "Program started": _exit() does not flush the stdio
// buffers, and when stdout is a pipe rather than a terminal it is block-buffered,
// so that line was still sitting in the buffer and never reached the pipe. Run it
// as ./ass3 and the line appears (a terminal is line-buffered); run it as
// ./ass3 | cat and it disappears.  !!!!
//
// Why atexit() is useful in practice
// ----------------------------------
// It keeps cleanup in one place instead of repeating it before every exit() in
// the program. A real program leaves through many paths and registering a
// handler once means every one of those paths does the cleanup.
//
// What it does not cover
// ----------------------
// The handler list is walked by exit(), and only by exit(). Anything that ends the
// process without reaching exit() skips it: _exit(), abort(), and termination by a
// signal such as SIGINT from Ctrl-C or SIGTERM from kill. A signalled process is torn
// down by the kernel without the program getting a say, so exit() is never entered.
// Status 143 (128 + SIGTERM) instead of a code of our own is the sign of this.
