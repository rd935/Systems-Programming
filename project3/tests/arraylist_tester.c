#include <stdio.h>
#include <stdlib.h>
#include "../arraylist.h"

int main(int argc, char** argv) {
    for (int i = 0; i < 2; i++) {
        // initialized empty arraylist
        arrlist* my_al = arr_create(1);
        arr_print(my_al);


        // after adding elements
        for (int j = 0; j < 26; j++) {
            char x[] = {97 + j, 97 + j, 97 + j, 97 + j, 97 + j, '\0'};
            arr_push(my_al, (char*) x);
        }
        arr_print(my_al);

        // tests if remove works, remove a random element at a time
        int len = my_al->len;
        for (int i = 0; i < len; i++) {
            int r = rand() % my_al->len;
            printf("%d\n", r);
            arr_remove(my_al, r);
            arr_print(my_al);
        }

        // deconstructing
        arr_destroy(my_al);
    }
}