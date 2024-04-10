#ifndef TDMM_H_
#define TDMM_H_
#define PAGE_SIZE 4096
#define HEADER_SIZE 32
#define BUDDY_PAGE_SIZE 4096 * 4096

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

metadata* buddySplit(metadata*);

metadata* searchBuddyFit(size_t);

metadata* newHeader(size_t, void*, metadata*, metadata*);

void insertHeader(metadata**, metadata**, metadata*);

void removeElement(metadata**, metadata**, metadata*);

void* createUsedBlock(metadata*, size_t);

void* firstFit(size_t);

void* bestFit(size_t);

void* worstFit(size_t);

void* buddyFit(size_t);

void t_init(alloc_strat_e, void*);

void* t_malloc(size_t);

void combineNonBuddy(metadata*, metadata*);

void coalesceNonBuddy(metadata*);

void coalesceBuddy(metadata*);

void t_free(void*);

void mark(void*);

void sweep();

void t_gcollect();

double get_memory_usage_percentage();

#endif