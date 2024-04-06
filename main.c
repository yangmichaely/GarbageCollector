#include "libtdmm/tdmm.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_SIZE 4096
#define NUM_ITERATIONS 260000

int main() {
    srand(time(NULL));

    // Initialize the allocator
    int a;
    t_init(FIRST_FIT, &a);

    // Allocate and immediately free a large number of blocks
    // for (int i = 0; i < NUM_ITERATIONS; i++) {
    //     size_t size = (rand() % MAX_SIZE) + 1;
    //     void* block = t_malloc(size);
    //     t_free(block);
    // }

    // // Allocate a large number of blocks, store the pointers, then free them
    // void** blocks = malloc(NUM_ITERATIONS * sizeof(void*));
    // for (int i = 0; i < NUM_ITERATIONS; i++) {
    //     size_t size = (rand() % MAX_SIZE) + 1;
    //     blocks[i] = t_malloc(size);
    // }
    // for (int i = 0; i < NUM_ITERATIONS; i++) {
    //     t_free(blocks[i]);
    // }
    //free(blocks);
    void* ptr1 = t_malloc(128);
    void* ptr2 = t_malloc(128);
    t_free(ptr1);
    void* ptr3 = t_malloc(128);
    t_free(ptr2);
    t_free(ptr3);
    return 0;
}