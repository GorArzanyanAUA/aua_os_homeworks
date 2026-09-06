// Objective: Work with pointers to characters and string manipulation.
// Task:
// Declare a string as a character array: char str[] = "Hello";.
// Declare a pointer to the first character of the string.
// Print the string using the pointer and a loop (without using str[i]).
// Count the number of characters in the string using pointer arithmetic.

#include <stdio.h>

int main() {
    char str[] = "Hello";
    char *c = str;

    while (*c){
        printf("%c", *c);
        c++;
    }
    printf("\n");

    int len = 0;
    c = str;
    while (*c){
        c++;
        len++;
    }
    printf("%d\n", len);
    return 0;
}
