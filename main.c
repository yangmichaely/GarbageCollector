#include "libtdmm/tdmm.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_SIZE 8 * 1024 * 1024

int main() {
    void* stack_bottom;
    printf("a: %p\n", &stack_bottom);
    t_init(FIRST_FIT, &stack_bottom);
    clock_t start;
    clock_t end;
    double time_taken;
    double average_time_taken = 0;
    size_t size = 1;
    FILE *fp;
    fp = fopen("/u/yangm/cs429/GarbageCollector/Report_Data/BUDDY.csv", "a");
    void** blocks[100];

    //OVERALL MEM_UTILIZATION
    // fprintf(fp, "\n%s    %s\n", "size(b)", "memUsage(percent)");
    // while(size <= MAX_SIZE){
    //     fprintf(fp, "%ld    ", size);
    //     void* block = t_malloc(size);
    //     fprintf(fp, "%lf\n", get_memory_usage_percentage());
    //     size *= 2;
    // }

    //OVERALL SPEEDS
    // int numReq = 0;
    // fprintf(fp, "\n%s    %s\n", "requests", "time(ns)");
    // start = clock();
    // for (int i = 0; i < 100; i++){
    //     numReq++;
    //     void* block = t_malloc(100);
    //     end = clock();
    //     time_taken = ((double)end - (double)start) * 1000000 / CLOCKS_PER_SEC;
    //     fprintf(fp, "%d    %lf\n", numReq, time_taken);

    //     numReq++;
    //     t_free(block);
    //     end = clock();
    //     time_taken = ((double)end - (double)start) * 1000000 / CLOCKS_PER_SEC;
    //     fprintf(fp, "%d    %lf\n", numReq, time_taken);
    // }

    //SPEEDS
    // fprintf(fp, "%s    %s    %s\n", "size(b)", "t_malloc(ns)", "t_free(ns)");
    // while(size <= MAX_SIZE){
    //     fprintf(fp, "%ld    ", size);
    //     start = clock();
    //     for(int i = 0; i < 100; i++){
    //         blocks[i] = t_malloc(size);
    //     }
    //     end = clock();
    //     //time is in ns
    //     time_taken = ((double)end - (double)start) * 1000000 / CLOCKS_PER_SEC;
    //     average_time_taken = time_taken / 100;
    //     fprintf(fp, "%f    ", average_time_taken);

    //     start = clock();
    //     for (int i = 0; i < 100; i++) {
    //         t_free(blocks[i]);
    //     }
    //     end = clock();
    //     time_taken = ((double)end - (double)start) * 1000000 / CLOCKS_PER_SEC;
    //     average_time_taken = time_taken / 100;
    //     fprintf(fp, "%f\n", average_time_taken);
    //     size *= 2;
    // }

    //OVERHEAD
    // fprintf(fp, "\n%s", "-----------MALLOC OVERHEAD-----------");
    // fprintf(fp, "\n%s    %s\n", "time(ns)", "overheadSize(b)");
    // start = clock();
    // for(int i = 0; i < 100; i++){
    //     blocks[i] = t_malloc(100);
    //     end = clock();
    //     time_taken = ((double)end - (double)start) * 1000000 / CLOCKS_PER_SEC;
    //     fprintf(fp, "%lf    %lu\n", time_taken, get_overhead());
    // }

    // fprintf(fp, "\n%s", "-----------FREE OVERHEAD-----------");
    // fprintf(fp, "\n%s    %s\n", "time(ns)", "overheadSize(b)");
    // start = clock();
    // for(int i = 0; i < 100; i++){
    //     t_free(blocks[i]);
    //     end = clock();
    //     time_taken = ((double)end - (double)start) * 1000000 / CLOCKS_PER_SEC;
    //     fprintf(fp, "%lf    %lu\n", time_taken, get_overhead());
    // }

    //MEM_UTILIZATION
    // fprintf(fp, "\n%s", "-----------MALLOC UTILIZATION-----------");
    // fprintf(fp, "\n%s    %s\n", "time(ns)", "memUtil(percent)");
    // start = clock();
    // for(int i = 0; i < 100; i++){
    //     blocks[i] = t_malloc(100);
    //     end = clock();
    //     time_taken = ((double)end - (double)start) * 1000000 / CLOCKS_PER_SEC;
    //     fprintf(fp, "%lf    %lf\n", time_taken, get_memory_usage_percentage());
    // }

    // fprintf(fp, "\n%s", "-----------FREE UTILIZATION-----------");
    // fprintf(fp, "\n%s    %s\n", "time(ns)", "memUtil(percent)");
    // start = clock();
    // for(int i = 0; i < 100; i++){
    //     t_free(blocks[i]);
    //     end = clock();
    //     time_taken = ((double)end - (double)start) * 1000000 / CLOCKS_PER_SEC;
    //     fprintf(fp, "%lf    %lf\n", time_taken, get_memory_usage_percentage());
    // }
    
    // fclose(fp);

    // GCOLLECT
    // void* ptr = t_malloc(100);
    // void* ptr2 = t_malloc(200);
    // ptr = t_malloc(150);
    // ptr = t_malloc(300);
    // ptr = t_malloc(400);
    // ptr = t_malloc(500);
    // ptr = t_malloc(600);
    // ptr = t_malloc(700);
    // printf("ptr: %p\n", &ptr);
    // printf("ptr2: %p\n", &ptr2);
    // t_gcollect();
    // printf("overhead: %lu\n", get_overhead());

    for (int i = 0; i < 5; i++){
        for(int j = 0; j < 100; j++){
            t_malloc(1000);
        }

        
        t_gcollect();
        printf("memUtil: %lf\n", get_memory_usage_percentage());
        printf("overhead: %lu\n", get_overhead());
    }

    return 0;
}