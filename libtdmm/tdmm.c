#include <stdlib.h>
#include <string.h>
#include <stdint.h> 
#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <float.h>
#include <sys/mman.h>
#include "tdmm.h" 

alloc_strat_e strat;
metadata* freeHead;
metadata* usedHead;
metadata* curFree;
metadata* curUsed;
uint64_t headerCounter;
void* stackBottom;
void* curPage;
void* memStart;
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
    insertHeader(&freeHead, &curFree, newFree);
    return block;
}

metadata* searchBuddyFit(size_t size){
    metadata* bestFit = searchBestFit(size);
    while(bestFit != NULL && bestFit -> size / 2 >= size && bestFit -> size / 2 >= MIN_BUDDY_SIZE){
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

void insertHeader(metadata** head, metadata** cur, metadata* cmp){
    metadata* temp = *head;
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
        if(*head != NULL && (*head) -> usableMem > cmp -> usableMem){
            cmp -> next = *head;
            cmp -> prev = NULL;
            (*head) -> prev = cmp;
            (*head) = cmp;
        }
        else if((*cur) != NULL && (*cur) -> usableMem < cmp -> usableMem){
            cmp -> next = NULL;
            (*cur) -> next = cmp;
            cmp -> prev = (*cur);
            (*cur) = cmp;
        }
        else{
            cmp -> next = NULL;
            cmp -> prev = NULL;
            (*head) = cmp;
            (*cur) = cmp;
        }
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
            insertHeader(&usedHead, &curUsed, block);
        }
        else{
            block -> size = size;
            metadata* next = block -> next;
            metadata* prev = block -> prev;
            metadata* newFree = newHeader(newSize, block -> usableMem + size, next, prev);
            removeElement(&freeHead, &curFree, block);
            insertHeader(&freeHead, &curFree, newFree);
            insertHeader(&usedHead, &curUsed, block);
        }
        return block -> usableMem;
    }
    else{
        void* newMem;
        if(size < PAGE_SIZE){
            newMem = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
            total_memory_allocated += PAGE_SIZE;
            metadata* newFree = newHeader(PAGE_SIZE - size, newMem + size, NULL, NULL);
            insertHeader(&freeHead, &curFree, newFree);
        }
        else{
            newMem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
            total_memory_allocated += size;
        }
        metadata* newUsed = newHeader(size, newMem, NULL, NULL);
        insertHeader(&usedHead, &curUsed, newUsed);
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

void* buddyFit(size_t size){
    metadata* buddyFit = searchBuddyFit(size);
    if(buddyFit != NULL){
        size = buddyFit -> size;
    }
    return createUsedBlock(buddyFit, size);
}

void t_init(alloc_strat_e allocStrat, void* stBot){
    strat = allocStrat;
    stackBottom = stBot;
    headerCounter = PAGE_SIZE;
    if(strat != BUDDY){
        void* usableMemory = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        total_memory_allocated += PAGE_SIZE;
        freeHead = newHeader(PAGE_SIZE, usableMemory, NULL, NULL);
    }
    else{
        void* usableMemory = mmap(NULL, BUDDY_PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        memStart = usableMemory;
        total_memory_allocated += BUDDY_PAGE_SIZE;
        freeHead = newHeader(BUDDY_PAGE_SIZE, usableMemory, NULL, NULL);
    }
    curFree = freeHead;
    curUsed = NULL;
    usedHead = NULL;
}

void* t_malloc(size_t size){
    if(size % 4 != 0){
        size += (4 - size % 4);
    }
    if(size > PAGE_SIZE){
        return createUsedBlock(NULL, size);
    }
    switch(strat){
        case FIRST_FIT:
            return firstFit(size);
        case BEST_FIT:
            return bestFit(size);
        case WORST_FIT:
            return worstFit(size);
        case BUDDY:
            return buddyFit(size);
    }
}

void combine(metadata* left, metadata* right){
    if(left != NULL && right != NULL && left -> usableMem + left -> size == right -> usableMem){
        if(right == curPage + headerCounter - HEADER_SIZE){
            headerCounter -= HEADER_SIZE;
        }
        left -> size += right -> size;
        if(right -> next != NULL){
            left -> next = right -> next;
            (right -> next) -> prev = left;
        }
        else{
            left -> next = NULL;
            curFree = left;
        }
        right = NULL;
        if(strat == BUDDY){
            coalesce(left);
        }
    }
}

void coalesce(metadata* block){
    metadata* next = block -> next;
    metadata* previous = block -> prev;
    if(strat != BUDDY){
        combine(block, next);
        combine(previous, block);
    }
    else{
        if(((block -> usableMem - memStart) / block -> size) % 2 == 0){
            if(next != NULL && block -> size == next -> size){
                combine(block, next);
            }
        }
        else{
            if(previous != NULL && block -> size == previous -> size){
                combine(previous, block);
            }
        }
    }
}

void t_free(void* ptr){
    metadata* temp = usedHead;
    while(temp != NULL){
        if(temp -> usableMem == ptr){
            memory_in_use -= temp -> size;
            removeElement(&usedHead, &curUsed, temp);
            insertHeader(&freeHead, &curFree, temp);
            coalesce(temp);
            break;
        }
        temp = temp -> next;
    }
}

void mark(void* p){
    if(p == NULL){
        return;
    }
    metadata* temp = usedHead;
    while(temp != NULL){
        //printf("p: %p, temp usablemem: %p\n", p, temp -> usableMem);
        if(temp -> usableMem <= p && temp -> usableMem + temp -> size > p){
            //printf("tempsize: %lu, p: %lu\n", temp -> usableMem, p);
            if(temp -> size % 4 == 0){
                //printf("1here:\n");
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
            //printf("2here:\n");
            memory_in_use -= temp -> size;
            removeElement(&usedHead, &curUsed, temp);
            insertHeader(&freeHead, &curFree, temp);
            coalesce(temp);
        }
        else{
            temp -> size--;
        }
        temp = temp -> next;
    }
}

void t_gcollect(){
    void* stackTop;
    for(uint64_t* i = (uint64_t*) &stackTop; i < (uint64_t*) stackBottom; i++){
        //printf("i: %p\n", i);
        mark(i);
    }
    metadata* temp = usedHead;
    while(temp != NULL){
        uint64_t* start_address = (uint64_t*) temp -> usableMem;
        for(uint64_t i = 0; i < temp -> size; i++){
            mark((void*)(start_address[i]));
            //printf("i: %lu\n", i);
        }
        temp = temp -> next;
    }
    sweep();
    // if(usedHead == NULL){
    //     printf("usedHead is NULL\n");
    // }
    // else{
    //     printf("usedHead is not NULL\n");
    // }
}

double get_memory_usage_percentage(){
    return ((double)memory_in_use / total_memory_allocated) * 100;
}