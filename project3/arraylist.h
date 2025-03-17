#ifndef ARRAYLIST_H
#define ARRAYLIST_H

typedef struct {
    char** h;
    unsigned len; 
    unsigned s;
} arrlist;

arrlist* arr_create(unsigned);
unsigned arr_length(arrlist *);
int arr_push(arrlist *, char*);
void arr_destroy(arrlist *);
char* arr_get(arrlist*, unsigned);
void arr_print(arrlist*);
int arr_contains(arrlist*, char*);
int arr_push(arrlist *, char*);
int arr_remove(arrlist*, int);

#endif