// Task 2: Using calloc for Initialized Array with Average Calculation
// Write a C program that:
// Asks the user for a number n.
// Dynamically allocates memory for an array of n integers using calloc() (so the array is zero-initialized).
// Lets the user input n integers.
// Calculates the average of the array.
// Prints the initial values (all zeroes) and the updated array.
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
	int *arr = calloc(n, sizeof(int));
	if (arr == NULL){
	    perror("calloc");
	    return 1;
	}

	printf("Initial Array\n");
	for (int i = 0; i < n; i++){
	    printf("%d ", arr[i]);
	}
	printf("\n");
	printf("Fill in the Array: ");
	for (int i = 0; i < n; i++){
		if (scanf("%d", &arr[i]) != 1){
			printf("Invalid integer\n");
			free(arr);
			return 1;
		}
	}

	int sum = 0;
	for (int i = 0; i < n; i++){
        	sum += arr[i];
        }
	printf("Average is %g\n", (double)sum/n);

        printf("Filled in Array\n");
        for (int i = 0; i < n; i++){
            printf("%d ", arr[i]);
        }
        printf("\n");

	free(arr);
	arr = NULL;
	return 0;
}
