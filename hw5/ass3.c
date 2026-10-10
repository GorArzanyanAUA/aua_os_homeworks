// Task 3: Resizing an Array Using realloc with Memory Shrinking
// Write a C program that:
// Dynamically allocates memory for an array of 10 integers using malloc().
// Lets the user input 10 values.
// Resizes the array using realloc() to only hold 5 integers (shrink the memory).
// Prints the first 5 values of the resized array.
// Frees the memory after use.

#include <stdio.h>
#include <stdlib.h>

#define INITIAL_SIZE 10
#define NEW_SIZE 5

int main(){
    int *arr = malloc(INITIAL_SIZE * sizeof(int));
    if (arr == NULL){
        perror("malloc");
        return 1;
    }

    printf("Enter %d integers: ", INITIAL_SIZE);
    for (int i = 0; i < INITIAL_SIZE; i++){
        if (scanf("%d", &arr[i]) != 1){
            printf("Invalid input\n");
            free(arr);
            return 1;
        }
    }
    
    // if realloc fails it returns NULL but the old block is still allocated, and
    // writing NULL straight into arr would lose the only pointer to it.
    int *tmp = realloc(arr, NEW_SIZE * sizeof(int));
    if (tmp == NULL){
        perror("realloc");
        free(arr);
        return 1;
    }
    arr = tmp;

    printf("Array after resizing: ");
    for (int i = 0; i < NEW_SIZE; i++)
        printf("%d ", arr[i]);
    printf("\n");

    free(arr);
    arr = NULL;
    return 0;
}

