#include <stdlib.h>
#include <string.h>
#include <stdint.h> 
#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <regex.h>
#include <float.h>
#include <sys/mman.h>
#include <errno.h>
#include "tdmm.h" 

alloc_strat_e strat;
metadata* freeHead;
metadata* usedHead;
metadata* curFree;
metadata* curUsed;
uint64_t headerCounter;
void* stackBottom;
void* curPage;
size_t total_memory_allocated = 0;
size_t memory_in_use = 0;

metadata* searchFirstFit(size_t size){
    metadata* temp = freeHead;
    while(temp != NULL){
        if(temp -> size >= size){
            return temp;
        }
        temp = temp -> next;
    }
    return NULL;
}

metadata* searchBestFit(size_t size){
    size_t minSize = SIZE_MAX;
    metadata* ans = NULL;
    metadata* temp = freeHead;
    while(temp != NULL){
        if(temp -> size >= size){
            if(temp -> size == size){
                return temp;
            }
            if(temp -> size < minSize){
                minSize = temp -> size;
                ans = temp;
            }
        }
        temp = temp -> next;
    }
    return ans;
}

metadata* searchWorstFit(size_t size){
    size_t maxSize = 0;
    metadata* ans = NULL;
    metadata* temp = freeHead;
    while(temp != NULL){
        if(temp -> size >= size){
            if(temp -> size > maxSize){
                maxSize = temp -> size;
                ans = temp;
            }
        }
        temp = temp -> next;
    }
    return ans;
}

metadata* buddySplit(metadata* block){
    block -> size /= 2;
    metadata* newFree = newHeader(block -> size, block -> usableMem + block -> size, NULL, NULL);
    insertFreeHeader(newFree);
    return block;
}

metadata* searchBuddyFit(size_t size){
    metadata* bestFit = searchBestFit(size);
    while(bestFit != NULL && bestFit -> size / 2 >= size){
        bestFit = buddySplit(bestFit);
    }
    return bestFit;
}

metadata* newHeader(size_t size, void* usableMem, metadata* next, metadata* prev){
    metadata* newHeader = NULL;
    if(headerCounter >= PAGE_SIZE){
        curPage = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        total_memory_allocated += PAGE_SIZE;
        newHeader = curPage;
        headerCounter = HEADER_SIZE;
    }
    else{
        newHeader = curPage + headerCounter;
        headerCounter += HEADER_SIZE;
    }
    newHeader -> size = size;
    newHeader -> usableMem = usableMem;
    newHeader -> next = next;
    newHeader -> prev = prev;
    return newHeader;
}

void insertFreeHeader(metadata* cmp){
    metadata* temp = freeHead;
    int found = 0;
    while(temp != NULL && temp -> next != NULL){
        if(temp -> usableMem < cmp -> usableMem && temp -> next -> usableMem > cmp -> usableMem){
            cmp -> next = temp -> next;
            temp -> next = cmp;
            cmp -> prev = temp;
            cmp -> next -> prev = cmp;
            found = 1;
            break;
        }
        temp = temp -> next;
    }
    if(found == 0){
        if(freeHead != NULL && freeHead -> usableMem > cmp -> usableMem){
            cmp -> next = freeHead;
            cmp -> prev = NULL;
            freeHead -> prev = cmp;
            freeHead = cmp;
        }
        else if(curFree != NULL && curFree -> usableMem < cmp -> usableMem){
            cmp -> next = NULL;
            curFree -> next = cmp;
            cmp -> prev = curFree;
            curFree = cmp;
        }
        else{
            cmp -> next = NULL;
            cmp -> prev = NULL;
            freeHead = cmp;
            curFree = cmp;
        }
    }
}

void insertUsedHeader(metadata* block){
    if(usedHead == NULL){
        usedHead = block;
        usedHead -> next = NULL;
        usedHead -> prev = NULL;
        curUsed = usedHead;
    }
    else{
        curUsed -> next = block;
        block -> prev = curUsed;
        curUsed = block;
        curUsed -> next = NULL;
    }
}

void removeElement(metadata** head, metadata** cur, metadata* block){
    metadata* previous = block -> prev;
    metadata* next = block -> next;
    if(next != NULL && previous != NULL){
        next -> prev = previous;
        previous -> next = next;
    }
    if(previous == NULL && next != NULL){
        (*head) = next;
        (*head) -> prev = NULL;
        next -> prev = NULL;
    }
    if(next == NULL && previous != NULL){
        (*cur) = previous;
        (*cur) -> next = NULL;
        previous -> next = NULL;
    }
    if(previous == NULL && next == NULL){
        (*head) = NULL;
        (*cur) = NULL;
    }
}

void* createUsedBlock(metadata* block, size_t size){
    memory_in_use += size;
    if(block != NULL){
        size_t newSize = block -> size - size;
        if(newSize == 0){
            removeElement(&freeHead, &curFree, block);
            insertUsedHeader(block);
        }
        else{
            block -> size = size;
            metadata* next = block -> next;
            metadata* prev = block -> prev;
            metadata* newFree = newHeader(newSize, block -> usableMem + size, next, prev);
            removeElement(&freeHead, &curFree, block);
            insertFreeHeader(newFree);
            insertUsedHeader(block);
        }
        return block -> usableMem;
    }
    else{
        void* newMem;
        if(size < PAGE_SIZE){
            newMem = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
            total_memory_allocated += PAGE_SIZE;
            metadata* newFree = newHeader(PAGE_SIZE - size, newMem + size, NULL, NULL);
            insertFreeHeader(newFree);
        }
        else{
            newMem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
            total_memory_allocated += size;
        }
        metadata* newUsed = newHeader(size, newMem, NULL, NULL);
        insertUsedHeader(newUsed);
        return newUsed -> usableMem;
    }
}

void* firstFit(size_t size){
    metadata* firstFit = searchFirstFit(size);
    return createUsedBlock(firstFit, size);
}

void* bestFit(size_t size){
    metadata* bestFit = searchBestFit(size);
    return createUsedBlock(bestFit, size);
}

void* worstFit(size_t size){
    metadata* worstFit = searchWorstFit(size);
    return createUsedBlock(worstFit, size);
}

void* buddy(size_t size){
    metadata* buddyFit = searchBuddyFit(size);
    if(buddyFit != NULL){
        size = buddyFit -> size;
    }
    return createUsedBlock(buddyFit, size);
}

void t_init(alloc_strat_e allocStrat, void* stBot){
    strat = allocStrat;
    stackBottom = stBot;
    void* usableMemory = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    total_memory_allocated += PAGE_SIZE;
    curPage = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    total_memory_allocated += PAGE_SIZE;
    freeHead = curPage;
    freeHead -> size = PAGE_SIZE;
    freeHead -> usableMem = usableMemory;
    freeHead -> next = NULL;
    freeHead -> prev = NULL;
    curFree = freeHead;
    curUsed = NULL;
    usedHead = NULL;
    headerCounter = HEADER_SIZE;
}

void* t_malloc(size_t size){
    if(size % 4 != 0){
        size += (4 - size % 4);
    }
    switch(strat){
        case FIRST_FIT:
            return firstFit(size);
        case BEST_FIT:
            return bestFit(size);
        case WORST_FIT:
            return worstFit(size);
        case BUDDY:
            return buddy(size);
    }
}

void combine(metadata* block){
    metadata* next = block -> next;
    metadata* previous = block -> prev;
    if(next != NULL && (strat != BUDDY || (strat == BUDDY && next -> size == block -> size))
    && block -> usableMem + block -> size == next -> usableMem){
        if(next == curPage + headerCounter - HEADER_SIZE){
            headerCounter -= HEADER_SIZE;
        }
        block -> size += next -> size;
        if(next -> next != NULL){
            block -> next = next -> next;
            (next -> next) -> prev = block;
        }
        else{
            block -> next = NULL;
            curFree = block;
        }
        next = NULL;
    }
    if(previous != NULL && (strat != BUDDY || (strat == BUDDY && previous -> size == block -> size))
    && previous -> usableMem + previous -> size == block -> usableMem){
        if(block == curPage + headerCounter - HEADER_SIZE){
            headerCounter -= HEADER_SIZE;
        }
        previous -> size += block -> size;
        if(block -> next != NULL){
            previous -> next = block -> next;
            (block -> next) -> prev = previous;
        }
        else{
            previous -> next = NULL;
            curFree = previous;
        }
        block = NULL;
    }
}

void t_free(void* ptr){
    metadata* temp = usedHead;
    while(temp != NULL){
        if(temp -> usableMem == ptr){
            memory_in_use -= temp -> size;
            removeElement(&usedHead, &curUsed, temp);
            insertFreeHeader(temp);
            combine(temp);
            break;
        }
        temp = temp -> next;
    }
}

void mark(void* p){
    metadata* temp = usedHead;
    while(temp != NULL){
        printf("p: %p, temp usablemem: %p\n", p, temp -> usableMem);
        if(temp -> usableMem <= p && temp -> usableMem + temp -> size > p){
            printf("tempsize: %d\n", temp -> size);
            
            if(temp -> size % 4 == 0){
                
                temp -> size++;
            }
            break;
        }
        temp = temp -> next;
    }
}

void sweep(){
    metadata* temp = usedHead;
    while(temp != NULL){
        if(temp -> size % 4 == 0){
            removeElement(&usedHead, &curUsed, temp);
            insertFreeHeader(temp);
            combine(temp);
        }
        else{
            temp -> size--;
        }
        temp = temp -> next;
    }
}

void t_gcollect(){
    void* stackTop;
    for(void* i = (void*) &stackTop; i < (void*) stackBottom; i++){
        printf("i: %p\n", i);
        mark(i);
    }
    metadata* temp = usedHead;
    while(temp != NULL){
        for(void* j = (void*) (temp -> usableMem); j < (void*) (temp -> usableMem + temp -> size); j += 8){
            mark(j);
        }
        temp = temp -> next;
    }
    sweep();
}

double get_memory_usage_percentage(){
    return ((double)memory_in_use / total_memory_allocated) * 100;
}