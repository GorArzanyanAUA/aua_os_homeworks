// Objective: Work with double pointers.
// Task:
// Declare an integer variable and a pointer to that variable.
// Declare a pointer to the pointer and initialize it.
// Print the value of the integer using both the pointer and the double-pointer.

#include <stdio.h>

int main() {
    int a = 10;
    int *p = &a;
    int **pp = &p;

    printf("%d\n", *p);
    printf("%d\n", **pp);

    return 0;
}
