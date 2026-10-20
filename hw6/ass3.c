// Create a struct with the following fields:
// char a;
// int b;
// double c;
// Without using #pragma pack, print the size of the struct and the addresses of each field.
// Add #pragma pack(1) before the struct definition and print the size and addresses again.
// Compare the two results and explain how #pragma pack affects memory usage.
// Bonus Task:
// Experiment with different packing values (e.g., #pragma pack(2), #pragma pack(4)), and note how the alignment changes.

#include <stdio.h>
#include <stddef.h>

// Default alignment.
struct Default {
    char a;
    int b;
    double c;
};

#pragma pack(push, 1)
struct Pack1 {
    char a;
    int b;
    double c;
};
#pragma pack(pop)

#pragma pack(push, 2)
struct Pack2 {
    char a;
    int b;
    double c;
};
#pragma pack(pop)

#pragma pack(push, 4)
struct Pack4 {
    char a;
    int b;
    double c;
};
#pragma pack(pop)

#pragma pack(push, 8)
struct Pack8 {
    char a;
    int b;
    double c;
};
#pragma pack(pop)

#define REPORT(label, type)                                                   \
    do {                                                                      \
        type s;                                                               \
        printf("%s: sizeof = %zu\n", label, sizeof(s));                       \
        printf("  &a = %p (offset %zu)\n", (void *)&s.a, offsetof(type, a));  \
        printf("  &b = %p (offset %zu)\n", (void *)&s.b, offsetof(type, b));  \
        printf("  &c = %p (offset %zu)\n", (void *)&s.c, offsetof(type, c));  \
    } while (0)

int main(void) {
    REPORT("Default", struct Default);
    REPORT("pack(1)", struct Pack1);
    REPORT("pack(2)", struct Pack2);
    REPORT("pack(4)", struct Pack4);
    REPORT("pack(8)", struct Pack8);

    // Explanation:
    // Default: a at 0, 3 bytes padding, b at 4, c at 8 (double needs 8-byte
    //   alignment) -> size 16. 3 bytes of padding are wasted.
    // pack(1): no padding at all: a at 0, b at 1, c at 5 -> size 13. Saves
    //   memory, but b and c are misaligned, which can be slower (or fault on
    //   some architectures).
    // pack(2): a at 0, 1 byte padding, b at 2, c at 6 -> size 14.
    // pack(4): a at 0, 3 bytes padding, b at 4, c at 8 -> size 16 (c is
    //   aligned to min(8, 4) = 4, which happens to land on 8 here).
    // pack(8): same as default on x86-64 -> size 16.
    // The struct is also padded at the end so its size is a multiple of its
    // (packed) alignment.
    return 0;
}

