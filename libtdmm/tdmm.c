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

metadata* searchFirstFit(size_t size){
    metadata* ans = NULL;
    metadata* temp = freeHead;
    while(temp != NULL){
        if(temp -> size >= size){
            return temp;
        }
        temp = temp -> next;
    }
}

metadata* searchBestFit(size_t size){
    int minSize = INT_MAX;
    metadata* ans = NULL;
    metadata* temp = freeHead;
    while(temp != NULL){
        if(temp -> size >= size){
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

void* newHeader(){
    metadata* newHeader = NULL;
    if(headerCounter > PAGE_SIZE){
        curPage = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        newHeader = curPage;
        headerCounter = HEADER_SIZE;
    }
    else{
        newHeader = curPage + headerCounter;
        headerCounter += HEADER_SIZE;
    }
    return newHeader;
}

void insertHeader(metadata* cmp){
    metadata* temp = freeHead;
    int found = 0;
    while(temp != NULL){
        if(temp -> usableMem > cmp -> usableMem && temp -> prev != NULL && (temp -> prev) -> usableMem < cmp -> usableMem){
            cmp -> next = temp;
            cmp -> prev = temp -> prev;
            if(cmp -> prev == NULL){
                freeHead = cmp;
            }
            else{
                (temp -> prev) -> next = cmp;
            }
            temp -> prev = cmp;
            found = 1;
            break;
        }
        temp = temp -> next;
    }
    if(found == 0){
        if(freeHead -> usableMem > cmp -> usableMem){
            cmp -> next = freeHead;
            freeHead -> prev = cmp;
            cmp -> prev = NULL;
            freeHead = cmp;
        }
        else if(curFree -> usableMem < cmp -> usableMem){
            curFree -> next = cmp;
            cmp -> prev = curFree;
            cmp -> next = NULL;
            curFree = cmp;
        }
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
            return block -> usableMem;
        }
        else{
            metadata* newUsed = newHeader();
            newUsed -> next = NULL;
            newUsed -> prev = NULL;
            newUsed -> size = size;
            newUsed -> usableMem = block -> usableMem;
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
            return newUsed -> usableMem;
        }
    }
    else{
        void* newMem;
        if(size < PAGE_SIZE){
            newMem = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
            metadata* newFree = newHeader();
            newFree -> next = NULL;
            newFree -> prev = NULL;
            newFree -> size = PAGE_SIZE - size;
            newFree -> usableMem = newMem + size;
            insertHeader(newFree);
        }
        else{
            newMem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        }
        metadata* newUsed = newHeader();
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
            newUsed -> next = NULL;
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

void t_init(alloc_strat_e allocStrat, void* stBot){
    strat = allocStrat;
    headerCounter = HEADER_SIZE;
    stackBottom = stBot;
    if(allocStrat != BUDDY){
        void* usableMemory = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        void* headerMemory = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        curPage = headerMemory;
        freeHead = (metadata*) headerMemory;
        freeHead -> size = PAGE_SIZE;
        freeHead -> usableMem = usableMemory;
        freeHead -> next = NULL;
        freeHead -> prev = NULL;
        curFree = freeHead;
        curUsed = NULL;
        usedHead = NULL;
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

metadata* combine(metadata* block){
    metadata* next = block -> next;
    metadata* previous = block -> prev;
    if(next != NULL && block -> usableMem + block -> size == next -> usableMem){
        block -> size += next -> size;
        block -> next = next -> next;
        if(next -> next != NULL){
            (next -> next) -> prev = block;
        }
        else{
            curFree = block;
        }
        next = NULL;
    }
    if(previous != NULL && previous -> usableMem + previous -> size == block -> usableMem){
        block -> size += previous -> size;
        block -> prev = previous -> prev;
        block -> usableMem = previous -> usableMem;
        if(previous -> prev != NULL){
            (previous -> prev) -> next = block;
        }
        else{
            freeHead = block;
        }
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
//     for(void* i = bottom; i < stackBottom - 8; i += 8){
//         //TODO: check every 8 bytes to see if they live on heap
//     }
}