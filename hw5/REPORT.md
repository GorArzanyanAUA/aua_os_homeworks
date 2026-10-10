# Homework 5 — Dynamic Memory Allocation

**Repository:** https://github.com/GorArzanyanAUA/aua_os_homeworks

**Author:** Gor Arzanyan


```
cd hw5
gcc ass1.c -o ../bin/ass1  
../bin/ass1
```


---

## Background — the four functions

| call | gives back | contents | on failure |
|---|---|---|---|
| `malloc(size)` | a block of `size` bytes | **uninitialized** (garbage) | `NULL` |
| `calloc(count, size)` | a block of `count * size` bytes | **all zero** | `NULL` |
| `realloc(ptr, size)` | a block of `size` bytes, possibly at a new address | first `min(old, new)` bytes copied | `NULL`, and `ptr` is **still valid** |
| `free(ptr)` | — | the block is returned to the allocator | `free(NULL)` does nothing |

Two habits are used in every program:

- **Every allocation is checked for `NULL`** before it is used.
- **`realloc`'s result goes into a temporary pointer first.** If `realloc` fails it returns
  `NULL` but does *not* free the old block.

The pointer is set to `NULL` after `free`, so any accidental later use crashes immediately
instead of silently touching freed memory.

---

## Task 1 — `malloc` and the sum of an array

```c
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
```

**How it works.** `malloc(n * sizeof(int))` reserves room for exactly `n` ints. The
memory is uninitialized, which is fine here because `scanf` fills every slot before anything
reads it. The sum is a `long` so that adding many large ints does not overflow as quickly.

**Output:**

```
Enter the number of elements: 5
Enter 5 integers: 1 2 3 4 5
Sum of the array: 15
```

---

## Task 2 — `calloc` and the average

```c
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
```

**How it works.** `calloc(n, sizeof(int))` takes the count and the element size separately
and zeroes the block, so printing it straight after the allocation is well defined and shows
all zeroes. Printing a `malloc`'d array at that point would read uninitialized memory.

The array is then filled from input, the average is printed, and the filled array is printed
again to show the change. The average casts to `double` *before* dividing. Without the cast
`50 / 4` is integer division and gives `12`, not `12.5`.

**Output:**

```
Enter the number of elements: 4
Initial Array
0 0 0 0
Fill in the Array: 5 10 15 20
Average is 12.5
Filled in Array
5 10 15 20
```

---

## Task 3 — shrinking with `realloc`

```c
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
```

**How it works.** `realloc(arr, 5 * sizeof(int))` keeps the first 5 values and releases
the rest. Shrinking usually happens in place (the address does not change), but the C
standard does not promise that, so the program always uses the returned pointer. After the
call, only `arr[0] .. arr[4]` belong to the program; reading `arr[5]` would be out of bounds,
even though the old values may still physically be there.

**Output:**

```
Enter 10 integers: 1 2 3 4 5 6 7 8 9 10
Array after resizing: 1 2 3 4 5
```

---

## Task 4 — an array of strings grown with `realloc`

```c
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
```

**How it works.** This uses two levels of allocation:

```
strs ──► [ ptr ][ ptr ][ ptr ]          ← malloc(3 * sizeof(char *))
            │      │      │
            ▼      ▼      ▼
         "Hello" "World"  "C"           ← malloc(51) each
```

- The array holds pointers, so it is allocated with `sizeof(char *)`, the size of one
  pointer. Each string is a separate allocation.
- Each string gets `MAX_LEN + 1 = 51` bytes: 50 characters and the terminating `'\0'`.
  `scanf("%50s", ...)` stops at whitespace and never writes past those 51 bytes.
- `realloc` grows **only the array of pointers** from 3 to 5. It copies the 3 existing
  pointers, so the strings they point to stay where they are and remain valid. The 2 new
  slots are then filled with fresh `malloc`s.
- Freeing goes in the opposite order: every string first, then the pointer array. Freeing
  the array first would lose the only pointers to the strings and leak them.

**Error paths.** Every failure frees exactly what has been allocated so far:

| failure | what is allocated | call |
|---|---|---|
| `malloc` for `strs[i]` fails | `strs[0] .. strs[i-1]` | `free_strings(strs, i)` |
| `scanf` into `strs[i]` fails | `strs[0] .. strs[i]` | `free_strings(strs, i + 1)` |
| `realloc` fails | the old array and its 3 strings | `free_strings(strs, INITIAL_COUNT)` |

When `realloc` fails, the old `strs` is still valid, which is why it can still be freed.

**Output:**

```
Enter 3 strings: Hello World C
Hello World C
Enter 2 more strings: Programming Memory
Hello World C Programming Memory
```


---

## Task 5 — student grades

```c
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
```

**How it works.** The highest and lowest are both seeded with `grades[0]`, not with `0` or
`100`. Seeding with a fixed number assumes a range: seeding the minimum with `0` would give a
wrong answer if every grade were above 0. The first real value is always a valid answer.

**Output:**

```
Enter the number of students: 5
Enter the grades: 90 85 70 60 95
Highest grade: 95
Lowest grade: 60
```

