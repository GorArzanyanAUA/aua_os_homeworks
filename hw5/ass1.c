// Task 1: Dynamic Integer Array with Sum Calculation
// Write a C program that:
// Asks the user for a number n.
// Dynamically allocates memory for an array of n integers using malloc().
// Lets the user input n integers.
// Calculates and prints the sum of the integers.
// Frees the memory after use.

#include <stdio.h>
#include <stdlib.h>

int main(){
    int n;
    printf("Enter the number of elements: ");
    if (scanf("%d", &n) != 1 || n <= 0){
        printf("Invalid number of elements\n");
        return 1;
    }
    
    int *arr = malloc(n * sizeof(int));
    if (arr == NULL){
        perror("malloc");
        return 1;
    }

    printf("Enter %d integers: ", n);
    for (int i = 0; i < n; i++){
        if (scanf("%d", &arr[i]) != 1){
            printf("Invalid input\n");
            free(arr);
            return 1;
        }
    }

    long sum = 0;
    for (int i = 0; i < n; i++)
        sum += arr[i];
    printf("Sum of the array: %ld\n", sum);

    free(arr);
    arr = NULL;
    return 0;
}

