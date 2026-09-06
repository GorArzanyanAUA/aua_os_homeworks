// Objective: Learn how pointer arithmetic works.
// Task:
// Declare an array of integers and initialize it with 5 values.
// Use a pointer to traverse the array and print each element.
// Modify the values of the array using pointer arithmetic.
// Print the modified array using both the pointer and the array name.

#include <stdio.h>

int main() {
        int arr[] = {10, 20, 30, 40, 50};
        int *ptr = arr;
        for (int i=0; i < 5; i++){
                printf("%d\n", *(ptr+i));
        }

        for (int i=0; i < 5; i++){
            *(ptr+i) += 100;
        }

        printf("\n");
        for (int i=0; i < 5; i++){
                printf("%d\n", *(ptr+i));
        }

        printf("\n");
        for (int i=0; i < 5; i++){
                printf("%d\n", *(arr+i));
        }


        return 0;
}

