// Objective: Understand the basics of pointers, the address-of operator, and dereferencing.
// Task:
// Declare an integer variable and initialize it with a value.
// Declare a pointer variable that points to the integer.
// Print the address of the integer variable using both the variable and the pointer.
// Modify the value of the integer using the pointer and print the new value.

#include <stdio.h>

int main() {
    int x = 10;
    int *ptr = &x;
    printf("%p\n", ptr);
    printf("%p\n", &x);
    
    *ptr = 11;
    printf("%d\n", x);

    return 0;
}
