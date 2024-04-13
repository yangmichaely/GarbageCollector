#include "libtdmm/tdmm.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_SIZE 1024 * 1024 * 8
#define NUM_ITERATIONS 25000

int main() {
    srand(time(NULL));

    // Initialize the allocator
    void* stack_bottom;
    printf("a: %p\n", &stack_bottom);
    t_init(FIRST_FIT, &stack_bottom);
    clock_t start;
    clock_t end;
    double time_taken;
    double average_time_taken = 0;
    size_t size;
    FILE *fp;
    fp = fopen("/u/yangm/cs429/GarbageCollector/Report_Data/FIRST_FIT.csv", "a");
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

    //SPEEDS
    // void** blocks[100];
    // size = 1;
    // fprintf(fp, "%s    %s    %s\n", "size(b)", "t_malloc avg (ns)", "t_free avg (ns)");
    // while(size <= MAX_SIZE){
    //     fprintf(fp, "%ld    ", size);
    //     start = clock();
    //     for(int i = 0; i < 100; i++){
    //         blocks[i] = t_malloc(size);
    //         //printf("malloc: %f\n", get_memory_usage_percentage());
    //     }
    //     end = clock();
    //     //time is in ns
    //     time_taken = ((double)end - (double)start) * 1000000 / CLOCKS_PER_SEC;
    //     average_time_taken = time_taken / 100;
    //     //printf("t_malloc total time: %f, t_malloc average time: %f\n", time_taken, average_time_taken);
    //     fprintf(fp, "%f    ", average_time_taken);

    //     start = clock();
    //     for (int i = 0; i < 100; i++) {
    //         t_free(blocks[i]);
    //         //printf("free: %f\n", get_memory_usage_percentage());
    //     }
    //     end = clock();
    //     time_taken = ((double)end - (double)start) * 1000000 / CLOCKS_PER_SEC;
    //     average_time_taken = time_taken / 100;
    //     //printf("t_free total time: %f, t_free average time: %f\n", time_taken, average_time_taken);
    //     fprintf(fp, "%f    \n", average_time_taken);
    //     size *= 2;
    // }
    //OVERHEAD
    // size = 1;
    // size_t sizeCounter = 0;
    // fprintf(fp, "\n%s    %s\n", "accumulated size(b)", "overhead size(b)");
    // while(size <= MAX_SIZE){
    //     fprintf(fp, "\n%s-----------------------\n\n", "MALLOC");
    //     for(int i = 0; i < 100; i++){
    //         sizeCounter += size;
    //         blocks[i] = t_malloc(size);
    //         fprintf(fp, "%lld    %lu\n", sizeCounter, get_overhead());
    //     }

    //     fprintf(fp, "\n%s-----------------------\n\n", "FREE");

    //     for (int i = 0; i < 100; i++) {
    //         sizeCounter -= size;
    //         t_free(blocks[i]);
    //         fprintf(fp, "%lld    %lu\n", sizeCounter, get_overhead());
    //     }

    //     size *= 2;
    // }

    fclose(fp);


    // for (int i = 0; i < NUM_ITERATIONS; i++) {
    //     void *ptr = t_malloc(10990909);
    //     t_free(ptr); // Assuming t_free is the corresponding free function
    // }

    void* ptr = t_malloc(100);
    void* ptr2 = t_malloc(200);
    ptr = t_malloc(150);
    ptr = t_malloc(300);
    ptr = t_malloc(400);
    ptr = t_malloc(500);
    ptr = t_malloc(600);
    ptr = t_malloc(700);
    printf("ptr: %p\n", &ptr);
    printf("ptr2: %p\n", &ptr2);
    t_gcollect();
    printf("overhead: %lu\n", get_overhead());
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