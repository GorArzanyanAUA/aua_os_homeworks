// Write a C program where you declare variables of the following types: int, char, double, and short.
// Use sizeof() to print the size of each data type.
// Print the addresses of each variable using the & operator.
// Observe and compare the addresses of these variables.
// Expected Output: Your program should show that variables are aligned to specific boundaries based on their types.

#include<stdio.h>
#include<stdint.h>
#include<inttypes.h>

int main() {
    int a = 15;
    char c = 'a';
    double d = 12;
    short s = 5;

    printf("size of char %ld, address %" PRIuPTR "\n", sizeof(c),  (uintptr_t)&c);
    printf("size of short %ld, address %" PRIuPTR "\n", sizeof(s), (uintptr_t)&s);
    printf("size of int %ld, address %" PRIuPTR "\n", sizeof(a),  (uintptr_t)&a);
    printf("size of double %ld, address %" PRIuPTR "\n", sizeof(d),  (uintptr_t)&d);

    // differences follow the order the variables are stored in, from lowest address to highest
    printf("\ndifference &s - &c: %td bytes\n", (char *)&s - (char *)&c);
    printf("difference &a - &s: %td bytes\n", (char *)&a - (char *)&s);
    printf("difference &d - &a: %td bytes\n", (char *)&d - (char *)&a);

    return 0;
}

// Observations:
// 1. No matter the declaration order, the variables are stored in the same order in memory:
//    char at the lowest address, then short, then int, then double at the highest.
//    This matches the description in the book: the largest variable comes first in the stack
//    frame (at the highest address, since the stack grows down), and each variable starts at
//    an address that is a multiple of its size (double % 8 == 0, int % 4 == 0, short % 2 == 0).
// 2. Storing them sorted by size minimizes the need for padding between them. In declaration
//    order (int, char, double, short) we would need 3 bytes of padding after char so that
//    double could start on an 8 byte boundary (18 bytes in total).
// 3. The layout is the compiler's choice (gcc here), not something the C standard requires.
//    A different compiler or other flags may place them differently, but the
//    alignment of each type stays the same.

