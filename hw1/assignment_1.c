#include <stdio.h>

int main() {
    int x = 10;
    int *ptr = &x;
    printf("%p\n", ptr);
    printf("%p\n", &x);
    
    *ptr = 11;
    printf("%d\n", x);

    return 0;
}