#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "arraylist.h"
#include <string.h>

#ifndef ARRL_DEBUG
#define ARRL_DEBUG 0
#endif

int arr_init(arrlist *, unsigned);

// create storage and initialize length/size
int arr_init(arrlist* L, unsigned cap)
{
    L->h = (char**) malloc(cap * sizeof(char*));
    
    if (L->h == NULL)
        return 0;

    L->len = 0;
    L->s = cap;
    return 1;
}

arrlist *arr_create(unsigned cap)
{
    arrlist* L = malloc(sizeof(arrlist));
    if (L == NULL)
        return NULL;

    if (arr_init(L, cap)) {
	    return L;
    }

    free(L);
    return NULL;
}

unsigned arr_length(arrlist *L)
{
    return L->len;
}

// add specified element to end of list
// assumes list has been initialized
int arr_push(arrlist *L, char* str)
{
    char* str_space = malloc(strlen(str) + 1);
    strcpy(str_space, str);
    str = str_space;

    // check whether array is full
    if (L->s == L->len) {
        int new_s = L->s * 2;

        char** new_h = realloc(L->h, new_s * sizeof(char*));
        
        if (new_h == NULL)
            return 0;

        if (ARRL_DEBUG)
            printf("Increased size to %d from %d\n", new_s, L->s);

        L->s = new_s;
        L->h = new_h;
    }

    L->h[L->len] = str;
    L->len++;

    return 1;
}

void arr_print (arrlist* L) {
    fprintf(stderr, "---\n");
    fprintf(stderr, "Length: %u\n", L->len);
    fprintf(stderr, "Size: %u\n", L->s);
    for (int i = 0; i < L->len; i++) {
        fprintf(stderr, "a[%d]: \t%p\t\t *a[%d]: \t;%s;\n", i, L->h[i], i, arr_get(L, i));
    }
    fprintf(stderr, "---\n");
}

int arr_contains(arrlist* L, char* str) {
    int matches = 0;
    for (int i = 0; i < L->len; i++) {
        if(strcmp(arr_get(L, i), str) == 0) {
            matches++;
        }
    }
    return matches;
}

// remove item from end of list
// write item to dest (if dest is non-NULL)
int arr_remove(arrlist *L, int index)
{
    if (L->len == 0)
        return 0;
    if (index >= L->len || index < 0) {
        return 0;
    }

    free(arr_get(L, index));

    for (int i = index + 1; i < L->len; i++) {
        L->h[i - 1] = L->h[i]; 
    }
    
    L->len--;
    
    if (ARRL_DEBUG)
        printf("Removed %d; new length %d\n", index, L->len);

    return 1;
}

void arr_destroy(arrlist *L)
{
    for(int i = 0; i < L->len; i++) {
        free(arr_get(L, i));
    }
    free(L->h);
    free(L);
}

char* arr_get(arrlist* L, unsigned index) {
    if(index >= L->len) {
        fprintf(stderr, "ERROR: Cannot access element %u of arraylist when length is %u\n", index, L->len);
        return NULL;
    } else {
        return L->h[index];
    }
}