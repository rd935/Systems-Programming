#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include "mymalloc.h"


#define MEMLENGTH 512
static double memory[MEMLENGTH];

void setBlockSize(char *start, int size, int allocate){
    int *block = (int*) start;
    *block = size;

    //printf("Block%d\n", *block);

    *(block + 1) = allocate;

    //printf("FLag%d\n", *(block + 1));    
}

int isFree(char* start){
    int memEnd = 4096;
    char *end = (char*) memory + memEnd;

    if(start < end){
        int flag = *(int*)(start + 4);
        
        if (flag == 1){
            //1 means that it is allocated
            return 1;
        }

        else{
            return 0;
        }
    }
    return 1;
    //printf("flag=%d", flag);

}


void mergeBlocks(char* start, char* merge){
    int* blockStart = (int*) start;
    //block size of start
    int blockSizeStart = *blockStart;

    int* blockMerge = (int*) merge;
    //block size of merge
    int blockSizeMerge = *blockMerge;

    *blockStart = blockSizeMerge + blockSizeStart;
    *(blockStart + 1) = 0;
    //clear out data
    *blockMerge = 0;
    *(blockMerge + 1) = 0;
}


int memCleared(){
    char* start = (char*) memory;
    int* block = (int*) start;
    int blockSize = *block;

    if((blockSize == 0 && isFree(start) == 0) || (blockSize == 8*MEMLENGTH && isFree(start) == 0)){
        return 1;
    }

    return 0;
}

void print(){
    char* start = (char*) memory;
    
    int i = 0;
    int memEnd = sizeof(start)*MEMLENGTH;

    printf("MEMEND = %d\n", memEnd);
    char* end = (char*) memory + memEnd;
    while (start < end){
        int* block = (int*) start;
        //int flag = *(block + 1);

        printf("index = %d\n", i);
        printf("Block Size = %d\n", *(block));
        printf("Allocation = %d\n", *(block + 1));

        start = start + 8;
        i++;
    }
}


void *mymalloc(size_t size, char *file, int line) {
    char *start = (char*) memory;

    //start pointer to integer pointer so size of memory is now 4 bytes
    //error check for 0 bytes

    if(size == 0){
        printf("Error: cannot allocate 0 bytes\n");
        printf("Error occured at line %d in file %s\n", line, file);
        return NULL;
    }

    size = ROUND8(size);

    int memEnd = sizeof(start)*MEMLENGTH;
    //error checking for asking for too much memory
    if (size > memEnd - 8){
        printf("ERROR: Too much memory\n");
        printf("Error occured at line %d in file %s\n", line, file);
        return NULL;
    }

    char *end = (char*) memory + memEnd;
    void *res = NULL;
    while (start < end){
        int* block = (int*) start;
        int blockSize = *block;
        
        //printf("%d\n", memEnd - (size + 8));
        //printf("i = %d\n", i);
        //printf("free = %d\n", free);
        //printf("%d", (int)(memEnd - (size + 8)));
        //printf("blockSize = %d\n", blockSize);
        //printf("size = %d", size);

        //first allocation

        //printf("END = %p\n START + SIZE + 8 = %p\n", &end, &(start) + size);
        //printf("SIZE = %ld\n MEMEND = %d\n", size, memEnd);

        if (blockSize == 0 && free == 0){
            setBlockSize(start, size + 8, 1);

            //printf("free = %d\n", free);
            //printf("blockSize = %d\n", blockSize);
            //printf("INT %d\n", *(int*)(start + size + 8));

            if ((start + size + 8) < end){
                setBlockSize((start + size + 8), (int)(memEnd - (size + 8)), 0);
            }

            res = start + 8;
            return res;
        }

        //allocation after first malloc
        if (free == 0 && blockSize >= (size + 8)){
            setBlockSize(start, size + 8, 1);

            res = start + 8;

            //printf("END = %s\n START + SIZE + 8 = %s\n", &end, &(start + size + 8));
            //printf("SIZE = %ld\n MEMEND = %d\n", size, memEnd);

            if((start + size + 8) < end){
                if (isFree(start + size + 8) == 0) {
                    //printf("%d", blockSize);
                    setBlockSize((start + size + 8), (int)(blockSize - (size + 8)), 0);
                }
            }

            return res;
        }

        //if the block is allocated or blockSize is small
        if (free == 1 || blockSize < (size + 8)){
            start = (start + blockSize);
            //printf("Next pointer \n");
        }
    }

    printf("Not enough memory\n");
    return NULL;
}


void myfree(void *ptr, char *file, int line){
    char* start = (char*) memory;
    int memEnd = sizeof(start) * MEMLENGTH;

    //checks to see if the pointer is invalid
    if (ptr == NULL || (int*) ptr < (int*) start || (int*) ptr > (int*) (start + memEnd)){
        printf("ERROR: Invalid pointer\n");
        printf("Error occured at line %d in file %s\n", line, file);
        return;
    }

    //previous block is only zero if there is nothing inside of the array
    int previousBlock = 0;

    //points to head of the block
    ptr = ptr - 8;

    //store address of header
    int* temp = (int*) ptr;

    //checks to see if allocation is 0, if 0 then that means
    //it is already freed or it is not initialized
    //int tempAllocation = *(temp + 1);
    
    if(*(temp + 1) == 0){
        //printf("%d\n", *temp);
        //printf("%d\n", *(temp + 1));

        printf("ERROR: ADDRESS NOT VALID\n");
        printf("Error occured at line %d in file %s\n", line, file);
        return;
    }

    char* end = (char*) memory + memEnd;
    while (start < end){
        int* block = (int*) start;
        int blockSize = *block;
        if(start == ptr){
            if (previousBlock != 0 && isFree((start - previousBlock)) == 0){
                //merges previous block and current block
                //start - previous block should lead to previous block
                mergeBlocks((start - previousBlock), start);

                //checks to see if the next block is also free
                if (!(start > end) && isFree((start + blockSize)) == 0){
                    //start + blockSize leads to next block
                    mergeBlocks((start - previousBlock), (start + blockSize));
                    ptr = NULL;
                    return;
                }

                //if not free then only merges previous block
                ptr = NULL;
                return;
            }

            //merges next block
            if (!(start > end) && isFree((start + blockSize)) == 0){
                //start + blockSize leads to next block mergeBlocks(start, (start + blockSize));

                ptr = NULL;
                return;
            }

            //could be more efficient

            setBlockSize(start, blockSize, 0);
            ptr = NULL;
            return;
        }

        //stores blockSize in previous before iterating
        previousBlock = blockSize;

        //iterate to next block
        start = start + blockSize;
    }

    printf("ERROR: Pointer Does Not Exist\n");
    printf("Error occurred at line %d in file %s\n", line, file);
    return;
}