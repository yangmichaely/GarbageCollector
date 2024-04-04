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
    uint64_t minSize = UINT64_MAX;
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
    uint64_t maxSize = 0;
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
            if(previous == NULL && next != NULL){
                freeHead = next;
            }
            if(next == NULL && previous != NULL){
                curFree = previous;
            }
            if(previous == NULL && next == NULL){
                freeHead = NULL;
                curFree = NULL;
            }
            block -> next = NULL;
            if(usedHead == NULL){
                block -> prev = NULL;
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
            metadata* newFree = newHeader();
            //newFree -> next = block -> next;
            //newFree -> prev = block -> prev;
            newFree -> next = NULL;
            newFree -> prev = NULL;
            newFree -> size = newSize;
            newFree -> usableMem = block -> usableMem + size;
            // if(curFree == block){
            //     curFree = newFree;
            // }
            // if(freeHead == block){
            //     freeHead = newFree;
            // }
            block -> size = size;
            block -> next = NULL;
            block -> prev = NULL;
            if(usedHead == NULL){
                usedHead = block;
            }
            else{
                curUsed -> next = block;
                block -> prev = curUsed;
            }
            curUsed = block;
            insertHeader(newFree);
        }
        return block -> usableMem;
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
        newUsed -> next = NULL;
        newUsed -> prev = NULL;
        if(usedHead == NULL){
            usedHead = newUsed;
        }
        else{
            curUsed -> next = newUsed;
            newUsed -> prev = curUsed;
        }
        curUsed = newUsed;
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

//TODO: BUDDY
// void* buddy(size_t size){
    
// }

void t_init(alloc_strat_e allocStrat, void* stBot){
    strat = allocStrat;
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
        headerCounter = HEADER_SIZE;
    }
    // else{
    //     uint8_t* buddy = (uint8_t*) headerMemory;

    // }
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

void combine(metadata* block){
    metadata* next = block -> next;
    metadata* previous = block -> prev;
    if(next != NULL && block -> usableMem + block -> size == next -> usableMem){
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
    if(previous != NULL && previous -> usableMem + previous -> size == block -> usableMem){
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
            metadata* previous = temp -> prev;
            metadata* next = temp -> next;
            if(previous != NULL){
                previous -> next = next;
                if(next == NULL){
                    curUsed = previous;
                }
            }
            if(next != NULL){
                next -> prev = previous;
                if(previous == NULL){
                    usedHead = next;
                }
            }
            if(previous == NULL && next == NULL){
                usedHead = NULL;
                curUsed = NULL;
            }
            insertHeader(temp);
            combine(temp);
            break;
        }
        temp = temp -> next;
    }
}

void t_gcollect(){
    void* stackTop;
    for(void* i = stackBottom; i < stackTop; i += 8){
        t_free(i);
    }
}