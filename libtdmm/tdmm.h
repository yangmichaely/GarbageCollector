#ifndef TDMM_H_
#define TDMM_H_
#define PAGE_SIZE 4096
#define HEADER_SIZE 32
#define MIN_BUDDY_SIZE sizeof(short)
#define MAX_BUDDY_SIZE PAGE_SIZE

#include <stddef.h>

typedef enum{
    FIRST_FIT,
    BEST_FIT,
    WORST_FIT,
    BUDDY
} alloc_strat_e;

typedef struct metadata metadata;

struct metadata{
    size_t size;
    void* usableMem;
    struct metadata* next;
    struct metadata* prev;
};

metadata* searchFirstFit(size_t);

metadata* searchBestFit(size_t);

metadata* searchWorstFit(size_t);

void* newHeader();

void insertHeader(metadata* cmp);

void* createUsedBlock(metadata*, size_t);

void* firstFit(size_t);

void* bestFit(size_t);

void* worstFit(size_t);

void* buddy(size_t);

void t_init(alloc_strat_e, void*);

void* t_malloc(size_t);

metadata* combine(metadata*);

void t_free(void*);

void t_gcollect(void);

#endif