// Objective: Learn how to pass pointers to functions.
// Task:
// Write a function swap(int *a, int *b) that swaps two integer values using pointers.
// In the main() function, call swap() and pass the addresses of two integers.
// Print the values of the integers before and after the swap.

#include <stdio.h>

void swap(int *a, int *b){
        int tmp = *a;
        *a = *b;
        *b = tmp;
}

int main() {
        int x = 10;
        int y = 11;

        printf("x = %d\n", x);
        printf("y = %d\n", y);

        swap(&x, &y);
        printf("\n");
        printf("x = %d\n", x);
        printf("y = %d\n", y);

        return 0;
}
