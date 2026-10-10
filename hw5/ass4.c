// Task 4: Dynamic String Array with realloc
// Write a C program that:
// Dynamically allocates memory for an array of 3 strings (char pointers) using malloc().
// Dynamically allocates memory for each string (each string can have up to 50 characters).
// Lets the user input 3 strings.
// Prints the strings.
// Uses realloc() to allow the user to input 2 more strings (resize the array).
// Prints all 5 strings.
// Frees all the allocated memory.

#include <stdio.h>
#include <stdlib.h>

#define MAX_LEN 50
#define INITIAL_COUNT 3
#define EXTRA_COUNT 2

void free_strings(char **strs, int count){
    for (int i = 0; i < count; i++)
        free(strs[i]);
    free(strs);
}

int main(){
    char **strs = malloc(INITIAL_COUNT * sizeof(char *));
    if (strs == NULL){
        perror("malloc");
        return 1;
    }

    printf("Enter %d strings: ", INITIAL_COUNT);
    for (int i = 0; i < INITIAL_COUNT; i++){
        strs[i] = malloc((MAX_LEN + 1) * sizeof(char)); // +1 for '\0'
        if (strs[i] == NULL){
            perror("malloc");
            free_strings(strs, i); // strs[0..i-1] are allocated, strs[i] is not
            return 1;
        }
        if (scanf("%50s", strs[i]) != 1){
            printf("Invalid input\n");
            free_strings(strs, i + 1); // strs[i] was allocated too, so free it
            return 1;
        }
    }

    for (int i = 0; i < INITIAL_COUNT; i++)
        printf("%s ", strs[i]);
    printf("\n");

    // Only the pointer array is resized. The 3 old pointers are copied over,
    // so the strings they point to stay where they are.
    char **tmp = realloc(strs, (INITIAL_COUNT + EXTRA_COUNT) * sizeof(char *));
    if (tmp == NULL){
        perror("realloc");
        free_strings(strs, INITIAL_COUNT); // strs is still valid when realloc fails
        return 1;
    }
    strs = tmp;

    printf("Enter %d more strings: ", EXTRA_COUNT);
    for (int i = INITIAL_COUNT; i < INITIAL_COUNT + EXTRA_COUNT; i++){
        strs[i] = malloc((MAX_LEN + 1) * sizeof(char));
        if (strs[i] == NULL){
            perror("malloc");
            free_strings(strs, i);
            return 1;
        }
        if (scanf("%50s", strs[i]) != 1){
            printf("Invalid input\n");
            free_strings(strs, i + 1);
            return 1;
        }
    }

    for (int i = 0; i < INITIAL_COUNT + EXTRA_COUNT; i++)
        printf("%s ", strs[i]);
    printf("\n");

    // Strings first, then the array of pointers.
    free_strings(strs, INITIAL_COUNT + EXTRA_COUNT);
    strs = NULL;
    return 0;
}

