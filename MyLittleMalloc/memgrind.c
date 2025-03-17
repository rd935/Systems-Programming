#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include "mymalloc.h"


#define NUM_TASKS 50
#define NUM_REPEATS 120


void task1() {
    for (int i = 0; i < NUM_REPEATS; i++){
        void *ptr = malloc(1);
        free(ptr);
    }
}

void task2() {
    void *ptrs[NUM_REPEATS];

    for (int i = 0; i < NUM_REPEATS; i++){
        ptrs[i]= malloc(1);
    }

    for (int i = 0; i < NUM_REPEATS; i++){
        free(ptrs[i]);
    }
}

void task3(){
    void *ptrs[NUM_REPEATS];
    int num_ptrs = 0;

    for (int i = 0; i < NUM_REPEATS; i++) {
        int choice = rand() % 2;

        if (choice == 0 && num_ptrs < NUM_REPEATS) {
            ptrs[num_ptrs++] = malloc(1);
        } 
        
        else if (num_ptrs > 0) {
            int idx = rand() % num_ptrs;
            free(ptrs[idx]);
            ptrs[idx] = ptrs[num_ptrs - 1];
            num_ptrs--;
        }
    }

    for (int i = 0; i < num_ptrs; i++) {
        free(ptrs[i]);
    }
}

void task4(){
    void **ptrs[NUM_REPEATS][NUM_REPEATS];

    for (int i = 0; i < NUM_REPEATS; i++){
        for (int j = 0; j < NUM_REPEATS; j++){
            ptrs[i][j] = malloc(1);
            free(ptrs[i][j]);
        }
    }
}

void task5(){
    void **ptrs[NUM_REPEATS][NUM_REPEATS];

    for (int i = 0; i < NUM_REPEATS; i++){
        for (int j = 0; j < NUM_REPEATS; j++){
            ptrs[i][j] = malloc(1);
        }
    }

    for (int i = 0; i < NUM_REPEATS; i++){
        for (int j = 0; j < NUM_REPEATS; j++){
            free(ptrs[i][j]);
        }
    }

}

int main() {
    srand(time(NULL));

    struct timeval start, end;
    long elapsed_time;

    // Task 1
    gettimeofday(&start, NULL);
    for (int i = 0; i < NUM_TASKS; i++) {
        task1();
    }
    gettimeofday(&end, NULL);
    elapsed_time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
    printf("Task 1 Average Time: %ld microseconds\n", elapsed_time / NUM_TASKS);

    // Task 2
    gettimeofday(&start, NULL);
    for (int i = 0; i < NUM_TASKS; i++) {
        task2();
    }
    gettimeofday(&end, NULL);
    elapsed_time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
    printf("Task 2 Average Time: %ld microseconds\n", elapsed_time / NUM_TASKS);

    // Task 3
    gettimeofday(&start, NULL);
    for (int i = 0; i < NUM_TASKS; i++) {
        task3();
    }
    gettimeofday(&end, NULL);
    elapsed_time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
    printf("Task 3 Average Time: %ld microseconds\n", elapsed_time / NUM_TASKS);

    // Task 4       
    gettimeofday(&start, NULL);
    for (int i = 0; i < NUM_TASKS; i++) {
        task4();
    }
    gettimeofday(&end, NULL);
    elapsed_time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
    printf("Task 4 Average Time: %ld microseconds\n", elapsed_time / NUM_TASKS);

    // Task 5
    gettimeofday(&start, NULL);
    for (int i = 0; i < NUM_TASKS; i++) {
        task5();
    }
    gettimeofday(&end, NULL);
    elapsed_time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
    printf("Task 5 Average Time: %ld microseconds\n", elapsed_time / NUM_TASKS);

    return 0;
}