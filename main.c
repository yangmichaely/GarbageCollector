#include "libtdmm/tdmm.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_SIZE 4096
#define NUM_ITERATIONS 25000

int main() {
    srand(time(NULL));

    // Initialize the allocator
    int a;
    printf("a: %d\n", a);
    t_init(BUDDY, &a);
    // clock_t start = clock();
    // // Allocate and immediately free a large number of blocks
    // for (int i = 0; i < NUM_ITERATIONS; i++) {
    //     size_t size = (rand() % MAX_SIZE) + 1;
    //     void* block = t_malloc(size);
    //     printf("malloc: %f\n", get_memory_usage_percentage());
    //     t_free(block);
    //     printf("free: %f\n", get_memory_usage_percentage());
    // }
    // clock_t end = clock();
    // double time_taken = ((double)end - (double)start) / CLOCKS_PER_SEC;
    // printf("t_free took %f seconds to execute \n", time_taken);
    // // Allocate a large number of blocks, store the pointers, then free them
    void** blocks = malloc(NUM_ITERATIONS * sizeof(void*));
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        size_t size = (rand() % MAX_SIZE) + 1;
        blocks[i] = t_malloc(size);
        printf("malloc: %f\n", get_memory_usage_percentage());
    }
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        t_free(blocks[i]);
        printf("free: %f\n", get_memory_usage_percentage());
    }
    free(blocks);
    // void* ptr1 = t_malloc(128);
    // printf("%f\n", get_memory_usage_percentage());
    // void* ptr2 = t_malloc(128);
    // printf("%f\n", get_memory_usage_percentage());
    // t_free(ptr1);
    // printf("%f\n", get_memory_usage_percentage());
    // void* ptr3 = t_malloc(128);
    // printf("%f\n", get_memory_usage_percentage());
    // t_free(ptr2);
    // printf("%f\n", get_memory_usage_percentage());
    // t_free(ptr3);
    // printf("%f\n", get_memory_usage_percentage());
    // int cnt = 1;
    // clock_t start = clock();
    // for(int i = 0; i < 1000; i++){
    //     for(int j = 0; j < 100; j++){
    //         t_malloc(1000);
    //     }
    //     //printf("%d--------------------------------------------------------\n", cnt++);
    //     t_gcollect();
    //     //printf("used: %f\n", get_memory_usage_percentage());
    // }
    // clock_t end = clock();
    // clock_t time_taken = ((double)end - start) / CLOCKS_PER_SEC;
    // printf("t_malloc took %f seconds to execute \n", time_taken);

    // void* ptr1 = t_malloc(128);
    // ptr1 = t_malloc(128);
    // t_free(ptr1);
    // t_gcollect();

    // clock_t start = clock();
    // void* block = t_malloc(100);
    // clock_t end = clock();
    // clock_t time_taken = ((double)end - start) / CLOCKS_PER_SEC;
    // printf("t_malloc took %f seconds to execute \n", time_taken);

    // start = clock();
    // t_free(block);
    // end = clock();
    // time_taken = ((double)end - start) / CLOCKS_PER_SEC;
    // printf("t_free took %f seconds to execute \n", time_taken);

    return 0;
}