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
int headerCounter;
void* stackTop;

metadata* searchFirstFit(size_t size){
    uint64_t minAddr = INT_MAX;
    metadata* ans = NULL;
    int found = 0;
    metadata* temp = freeHead;
    while(temp != NULL){
        if(temp -> size >= size){
            found = 1;
            if((uint64_t) temp -> usableMem < minAddr){
                minAddr = (uint64_t) temp -> usableMem;
                ans = temp;
            }
        }
        temp = temp -> next;
    }
    return ans;
}

metadata* searchBestFit(size_t size){
    int minSize = INT_MAX;
    metadata* ans = NULL;
    int found = 0;
    metadata* temp = freeHead;
    while(temp != NULL){
        if(temp -> size >= size){
            found = 1;
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
    int maxSize = -1;
    metadata* ans = NULL;
    int found = 0;
    metadata* temp = freeHead;
    while(temp != NULL){
        if(temp -> size >= size){
            found = 1;
            if(temp -> size > maxSize){
                maxSize = temp -> size;
                ans = temp;
            }
        }
        temp = temp -> next;
    }
    return ans;
}

void* newHeader(metadata* block){
    headerCounter += HEADER_SIZE;
    metadata* newHeader;
    if(headerCounter > PAGE_SIZE){
        newHeader = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        headerCounter = HEADER_SIZE;
    }
    else{
        newHeader = block;
    }
    return newHeader;
}

void insertHeader(metadata* cmp){
    metadata* temp = freeHead;
    int found = 0;
    while(temp -> next != NULL){
        if(temp -> usableMem > cmp -> usableMem){
            cmp -> next = temp;
            cmp -> prev = temp -> prev;
            if(cmp -> prev == NULL){
                freeHead = cmp;
            }
            else{
                ((metadata*) (temp -> prev)) -> next = cmp;
            }
            temp -> prev = cmp;
            found = 1;
            break;
        }
        temp = temp -> next;
    }
    if(found == 0){
        temp -> next = cmp;
        cmp -> prev = temp;
        cmp -> next = NULL;
        curFree = cmp;
    }
}

void* createUsedBlock(metadata* block, size_t size){
    if(block != NULL){
        int newSize = block -> size - size;
        if(newSize == 0){
            metadata* previous = block -> prev;
            metadata* next = block -> next;
            if(previous != NULL){
                previous -> next = next;
            }
            if(next != NULL){
                next -> prev = previous;
            }
            if(previous == NULL){
                freeHead = next;
            }
            if(next == NULL){
                curFree = previous;
            }
            block -> next = NULL;
            if(usedHead == NULL){
                usedHead = block;
                curUsed = block;
            }
            else{
                curUsed -> next = block;
                block -> prev = curUsed;
                curUsed = block;
            }
        }
        else{
            metadata* newUsed = newHeader(block);
            newUsed -> next = NULL;
            newUsed -> prev = NULL;
            newUsed -> size = size;
            if(usedHead == NULL){
                usedHead = newUsed;
                curUsed = newUsed;
            }
            else{
                curUsed -> next = newUsed;
                newUsed -> prev = curUsed;
                curUsed = newUsed;
            }
            block -> size = newSize;
            block -> usableMem = block -> usableMem + size;
        }
        return block -> usableMem;
    }
    else{
        void* newMem;
        if(size < PAGE_SIZE){
            newMem = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
            metadata* newFree = newHeader(newMem + size);
            newFree -> size = PAGE_SIZE - size;
            newFree -> usableMem = newMem + size;
            insertHeader(newFree);
        }
        else{
            newMem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        }
        metadata* newUsed = newHeader(newMem);
        newUsed -> usableMem = newMem;
        newUsed -> size = size;
        if(usedHead == NULL){
            newUsed -> next = NULL;
            newUsed -> prev = NULL;
            usedHead = newUsed;
            curUsed = newUsed;
        }
        else{
            curUsed -> next = newUsed;
            newUsed -> prev = curUsed;
            curUsed = newUsed;
        }
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

// //TODO: BUDDY
// void* buddy(size_t size){
    
// }

void t_init(alloc_strat_e allocStrat, void* stTop){
    if(allocStrat != BUDDY){
        void* usableMemory = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        void* headerMemory = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        freeHead = (metadata*) headerMemory;
        freeHead -> size = PAGE_SIZE;
        freeHead -> usableMem = usableMemory;
        freeHead -> next = NULL;
        freeHead -> prev = NULL;
        curFree = freeHead;
        curUsed = NULL;
        usedHead = NULL;
        strat = allocStrat;
        headerCounter = HEADER_SIZE;
        stackTop = stTop;
    }
}

void* t_malloc(size_t size){
    switch(strat){
        case FIRST_FIT:
            return firstFit(size);
        case BEST_FIT:
            return bestFit(size);
        case WORST_FIT:
            return worstFit(size);
        // case BUDDY:
        //     return buddy(size);
    }
}

void* combine(metadata* block){
    metadata* next = block -> next;
    metadata* previous = block -> prev;
    if(next != NULL && (uint64_t) block -> usableMem + block -> size == (uint64_t) next -> usableMem){
        block -> size += next -> size;
        block -> next = next -> next;
        if(next -> next != NULL){
            ((metadata*) (next -> next)) -> prev = block;
        }
        else{
            curFree = block;
        }
        next = NULL;
    }
    if(previous != NULL && (uint64_t) previous -> usableMem + previous -> size == (uint64_t) block -> usableMem){
        block -> size += previous -> size;
        block -> prev = previous -> prev;
        if(previous -> prev != NULL){
            ((metadata*) (previous -> prev)) -> next = block;
        }
        else{
            freeHead = block;
        }
        block -> usableMem = previous -> usableMem;
        previous = NULL;
    }
    return block;
}

void t_free(void* ptr){
    metadata* temp = usedHead;
    while(temp != NULL){
        if(temp -> usableMem == ptr){
            metadata* previous = temp -> prev;
            metadata* next = temp -> next;
            if(previous != NULL){
                previous -> next = next;
            }
            if(next != NULL){
                next -> prev = previous;
            }
            if(previous == NULL){
                usedHead = next;
            }
            if(next == NULL){
                curUsed = previous;
            }
            insertHeader(temp);
            temp = combine(temp);
            break;
        }
        temp = temp -> next;
    }
}

//TODO: implement this
void t_gcollect(void){
//     void* bottom;
//     for(void* i = bottom; i < stackTop - 8; i += 8){
//         //TODO: check every 8 bytes to see if they live on heap
//     }
}