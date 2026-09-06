# Homework 1 — Pointers in C

**Repository:** https://github.com/GorArzanyanAUA/aua_os_homeworks

**Author:** Gor Arzanyan

All five programs compile with `gcc` on the server without warnings.

---

## Assignment 1 — Basics of Pointers

```c
int x = 10;
int *ptr = &x;
```

`&x` gives the address of `x`, stored in `ptr`. Writing `*ptr = 11` follows that address and changes `x` itself.

**Output:** the two addresses printed are identical, then `11`.

**Observation:** `ptr` doesn't hold a copy of `x` — it holds where `x` lives. That's why changing `*ptr` changes `x`.

---

## Assignment 2 — Pointer Arithmetic

```c
int arr[] = {10, 20, 30, 40, 50};
int *ptr = arr;
*(ptr+i) += 100;
```

An array name used in an expression becomes a pointer to its first element, so `ptr = arr` needs no `&`. `*(ptr+i)` reads the element `i` steps along.

**Output:** `10 20 30 40 50`, then `110 120 130 140 150` printed twice — once through `ptr`, once through `arr`.

**Observations:**
- `ptr+1` moves forward by one `int` (4 bytes here), not one byte. The compiler scales the step by the type.
- `*(ptr+i)` and `arr[i]` mean exactly the same thing — indexing is just shorthand for pointer arithmetic.
---

## Assignment 3 — Passing Pointers to Functions

```c
void swap(int *a, int *b){
    int tmp = *a;
    *a = *b;
    *b = tmp;
}
swap(&x, &y);
```

Passing addresses lets `swap` write back into `main`'s variables.

**Output:** `x = 10, y = 11` becomes `x = 11, y = 10`.

---

## Assignment 4 — Double Pointers

```c
int a = 10;
int *p = &a;
int **pp = &p;
```

A pointer is a variable too, so it has its own address. `pp` stores the address of `p`. Each `*` follows one link: `*pp` is `p`, and `**pp` is `a`.

**Output:** `10` twice.

**Observation:** there are three variables forming a chain `pp → p → a`, and dereferencing walks it one step at a time. The number of `*`s in the type tells you how many steps back to the `int`.

---

## Assignment 5 — Character Pointers and Strings

```c
char str[] = "Hello";
char *c = str;

while (*c){ printf("%c", *c); c++; }

int len = 0;
c = str;
while (*c){ c++; len++; }
```

`"Hello"` takes 6 bytes — 5 letters plus a `'\0'` terminator. Both loops walk forward until they hit that zero byte.

**Output:** `Hello`, then `5`.

**Observations:**
- I first wrote `while (c)` instead of `while (*c)`. That tests the pointer (never zero) instead of the character, so the loop ran past the end of the string. Dereferencing is what makes the terminator check work.
- Length is 5 but storage is 6 bytes — the terminator isn't counted.

