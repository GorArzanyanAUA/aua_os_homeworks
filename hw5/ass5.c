// Task 5: Dynamic Memory for Student Grades
// Write a C program that:
// Asks the user how many students are in a class.
// Dynamically allocates memory to store the grades of each student using malloc().
// Lets the user input the grade for each student.
// Calculates and prints the highest and lowest grades in the class.
// Frees the memory after use.

#include <stdio.h>
#include <stdlib.h>

int main(){
    int n;
    printf("Enter the number of students: ");
    if (scanf("%d", &n) != 1 || n <= 0){
        printf("Invalid number of students\n");
        return 1;
    }
    int *grades = malloc(n * sizeof(int));
    if (grades == NULL){
        perror("malloc");
        return 1;
    }

    printf("Enter the grades: ");
    for (int i = 0; i < n; i++){
        if (scanf("%d", &grades[i]) != 1){
            printf("Invalid input\n");
            free(grades);
            return 1;
        }
    }

    int highest = grades[0];
    int lowest = grades[0];
    for (int i = 1; i < n; i++){
        if (grades[i] > highest)
            highest = grades[i];
        if (grades[i] < lowest)
            lowest = grades[i];
    }

    printf("Highest grade: %d\n", highest);
    printf("Lowest grade: %d\n", lowest);

    free(grades);
    grades = NULL;
    return 0;
}
