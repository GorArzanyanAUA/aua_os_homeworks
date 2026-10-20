// Create a struct in C with the following fields in the given order:
// char a;
// int b;
// double c;
// Use sizeof() to print the size of the struct and print the addresses of each field using the & operator.
// Change the order of the fields (for example, place double first, then int, then char) and observe the size of the struct again.
// Answer the following questions:
// Does changing the order of fields reduce or increase the size of the struct? Why?
// How is memory padding affecting the size of your struct?


#include<stdio.h>
#include<stdint.h>
#include<inttypes.h>

struct S1 {
    char a;
    int b;
    double c;
};

struct S2 {
    double c;
    int b;
    char a;
};

struct S3 {
    char a;
    double c;
    int b;
};

int main() {
    struct S1 x;
    struct S2 y;
    struct S3 z;

    printf("struct S1 (char, int, double): size %zu\n", sizeof(struct S1));
    printf("  &a %" PRIuPTR " (offset %td)\n", (uintptr_t)&x.a, (char *)&x.a - (char *)&x);
    printf("  &b %" PRIuPTR " (offset %td)\n", (uintptr_t)&x.b, (char *)&x.b - (char *)&x);
    printf("  &c %" PRIuPTR " (offset %td)\n", (uintptr_t)&x.c, (char *)&x.c - (char *)&x);

    printf("\nstruct S2 (double, int, char): size %zu\n", sizeof(struct S2));
    printf("  &c %" PRIuPTR " (offset %td)\n", (uintptr_t)&y.c, (char *)&y.c - (char *)&y);
    printf("  &b %" PRIuPTR " (offset %td)\n", (uintptr_t)&y.b, (char *)&y.b - (char *)&y);
    printf("  &a %" PRIuPTR " (offset %td)\n", (uintptr_t)&y.a, (char *)&y.a - (char *)&y);

    printf("\nstruct S3 (char, double, int): size %zu\n", sizeof(struct S3));
    printf("  &a %" PRIuPTR " (offset %td)\n", (uintptr_t)&z.a, (char *)&z.a - (char *)&z);
    printf("  &c %" PRIuPTR " (offset %td)\n", (uintptr_t)&z.c, (char *)&z.c - (char *)&z);
    printf("  &b %" PRIuPTR " (offset %td)\n", (uintptr_t)&z.b, (char *)&z.b - (char *)&z);

    printf("\nsum of the field sizes: %zu\n", sizeof(char) + sizeof(int) + sizeof(double));

    return 0;
}

// Answers:
// 1. The order of the fields can change the size. S1 (char, int, double) and S2 (double, int, char)
//    are 16 bytes, but S3 (char, double, int) is 24 bytes, although the fields are the same and
//    add up to only 13 bytes. So a bad order increases the size, a good one keeps it small.
// 2. The governing principle is that every field must start at an address that is a multiple of its size, 
//    and the whole struct size must be a multiple of the largest alignment (8 here).
//    S1: a at offset 0 (1 byte), 3 bytes of padding so b is on a 4 byte boundary (offset 4),
//        c at offset 8 (already a multiple of 8). Padding is in the middle: 1 + 3 + 4 + 8 = 16.
//    S2: c at offset 0, b at offset 8, a at offset 12, no padding between the fields.
//        3 bytes of tail padding make the size a multiple of 8: 8 + 4 + 1 + 3 = 16.
//    Tail padding is needed so that in an array of structs every element stays aligned.
//    S3: a at offset 0, 7 bytes of padding so c is on an 8 byte boundary (offset 8), b at offset 16,
//        4 bytes of tail padding to make the size a multiple of 8: 1 + 7 + 8 + 4 + 4 = 24.
//    S1 and S2 waste 3 bytes, S3 wastes 11 bytes, only because of the order of the fields.
//    Putting the fields from the largest to the smallest (S2) gives the least padding.

