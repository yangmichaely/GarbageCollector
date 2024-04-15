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
buddyNode* buddyHead;
buddyNode* curBuddy;
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

// metadata* buddySplit(metadata* block){
//     block -> size /= 2;
//     metadata* newFree = newHeader(block -> size, block -> usableMem + block -> size);
//     insertHeader(&freeHead, &curFree, newFree);
//     return block;
// }

void* searchBuddyFit(size_t size){
    buddyNode* temp = buddyHead;
    while(temp != NULL){
        for(int i = 0; i < BUDDY_PAGE_SIZE / MIN_BUDDY_SIZE - size; i++){
            int counter = 0;
            for(int j = 0; j < size; j++){
                if(temp -> buddyMap[i + j] == 1){
                    break;
                }
                else{
                    counter++;
                }
            }
            if(counter == size){
                for(int j = 0; j < size; j++){
                    temp -> buddyMap[i + j] = 1;
                }
                metadata* newUsed = newHeader(size * MIN_BUDDY_SIZE, temp -> usableMem + i * MIN_BUDDY_SIZE);
                insertHeader(&usedHead, &curUsed, newUsed);
                return temp -> usableMem + i * MIN_BUDDY_SIZE;
            }
        }
        temp = temp -> next;
    }
    return NULL;
}

metadata* newHeader(size_t size, void* usableMem){
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
    newHeader -> next = NULL;
    newHeader -> prev = NULL;
    return newHeader;
}

buddyNode* newBuddyHeader(uint8_t* bMap, void* usableMem){
    buddyNode* newHeader = NULL;
    if(headerCounter >= PAGE_SIZE){
        curPage = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        total_memory_allocated += PAGE_SIZE;
        newHeader = curPage;
        headerCounter = 24;
    }
    else{
        newHeader = curPage + headerCounter;
        headerCounter += 24;
    }
    newHeader -> buddyMap = bMap;
    newHeader -> usableMem = usableMem;
    newHeader -> next = NULL;
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
            (*head) -> prev = cmp;
            cmp -> prev = NULL;
            (*head) = cmp;
        }
        else if((*cur) != NULL && (*cur) -> usableMem < cmp -> usableMem){
            (*cur) -> next = cmp;
            cmp -> prev = (*cur);
            cmp -> next = NULL;
            (*cur) = cmp;
        }
        else{
            cmp -> next = NULL;
            cmp -> prev = NULL;
            (*head) = cmp;
            (*cur) = cmp;
        }
    }
    // if(*cur == NULL){
    //     cmp -> prev = NULL;
    //     cmp -> next = NULL;
    //     *head = cmp;
    //     *cur = cmp;
    // }
    // else{
    //     (*cur) -> next = cmp;
    //     cmp -> prev = *cur;
    //     *cur = cmp;
    //     cmp -> next = NULL;
    // }
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
            metadata* newFree = newHeader(newSize, block -> usableMem + size);
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
            metadata* newFree = newHeader(PAGE_SIZE - size, newMem + size);
            insertHeader(&freeHead, &curFree, newFree);
        }
        else{
            newMem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
            total_memory_allocated += size;
        }
        metadata* newUsed = newHeader(size, newMem);
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
    uint64_t bmapSize = size / MIN_BUDDY_SIZE;
    void* status = searchBuddyFit(bmapSize);
    if(status != NULL){
        return status;
    }
    void* usableMemory = mmap(NULL, BUDDY_PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    uint8_t* bMap = mmap(NULL, BUDDY_PAGE_SIZE / MIN_BUDDY_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    for(int i = 0; i < BUDDY_PAGE_SIZE / MIN_BUDDY_SIZE; i++){
        if(i < bmapSize){
            bMap[i] = 1;
        }
        else{
            bMap[i] = 0;
        }
    }
    buddyNode* temp = newBuddyHeader(bMap, usableMemory);
    curBuddy -> next = temp;
    curBuddy = temp;
    total_memory_allocated += (BUDDY_PAGE_SIZE + BUDDY_PAGE_SIZE / MIN_BUDDY_SIZE);
    metadata* newUsed = newHeader(size, temp -> usableMem);
    insertHeader(&usedHead, &curUsed, newUsed);
    return newUsed -> usableMem;
}

void t_init(alloc_strat_e allocStrat, void* stBot){
    strat = allocStrat;
    stackBottom = stBot;
    headerCounter = PAGE_SIZE;
    if(strat != BUDDY){
        void* usableMemory = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        total_memory_allocated += PAGE_SIZE;
        freeHead = newHeader(PAGE_SIZE, usableMemory);
    }
    else{
        void* usableMemory = mmap(NULL, BUDDY_PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        uint8_t* bMap = mmap(NULL, BUDDY_PAGE_SIZE / MIN_BUDDY_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        for(int i = 0; i < BUDDY_PAGE_SIZE / MIN_BUDDY_SIZE; i++){
            bMap[i] = 0;
        }
        buddyHead = newBuddyHeader(bMap, usableMemory);
        curBuddy = buddyHead;
        //memStart = usableMemory;
        total_memory_allocated += (BUDDY_PAGE_SIZE + BUDDY_PAGE_SIZE / MIN_BUDDY_SIZE);
        //freeHead = newHeader(BUDDY_PAGE_SIZE, usableMemory);
    }
    curFree = freeHead;
    curUsed = NULL;
    usedHead = NULL;
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
            if(size % 16 != 0){
                size += (16 - size % 16);
            }
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
    if(strat != BUDDY){
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
    else{
        metadata* usedTemp = usedHead;
        size_t size;
        while(usedTemp != NULL){
            if(ptr == usedTemp -> usableMem){
                size = usedTemp -> size;
                break;
            }
            usedTemp = usedTemp -> next;
        }
        buddyNode* temp = buddyHead;
        while(temp != NULL){
            if(ptr >= temp -> usableMem && ptr < temp -> usableMem + BUDDY_PAGE_SIZE){
                int index = (ptr - temp -> usableMem) / MIN_BUDDY_SIZE;
                for(int i = 0; i < size / MIN_BUDDY_SIZE; i++){
                    temp -> buddyMap[index + i] = 0;
                }
                break;
            }
            temp = temp -> next;
        }
        removeElement(&usedHead, &curUsed, usedTemp);
    }
}

void markHeap(void* start, void* end){
    for(void* i = start; i < end; i++){
        metadata* temp = usedHead;
        while(temp != NULL){
            if(i >= temp -> usableMem && i < temp -> usableMem + temp -> size){
                if(temp -> size % 4 == 0){
                    temp -> size++;
                    markHeap(temp -> usableMem, temp -> usableMem + temp -> size);
                }
                break;
            }
            temp = temp -> next;
        }
    }
}

void sweep(){
    metadata* temp = usedHead;
    metadata* next = temp;
    while(next != NULL){
        next = temp -> next;
        if(temp -> size % 4 == 0){
            memory_in_use -= temp -> size;
            if(strat != BUDDY){
                removeElement(&usedHead, &curUsed, temp);
                insertHeader(&freeHead, &curFree, temp);
                coalesce(temp);
            }
            else{
                buddyNode* buddyTemp = buddyHead;
                while(buddyTemp != NULL){
                    if(temp -> usableMem >= buddyTemp -> usableMem && temp -> usableMem < buddyTemp -> usableMem + BUDDY_PAGE_SIZE){
                        int index = (temp -> usableMem - buddyTemp -> usableMem) / MIN_BUDDY_SIZE;
                        for(int i = 0; i < temp -> size / MIN_BUDDY_SIZE; i++){
                            buddyTemp -> buddyMap[index + i] = 0;
                        }
                        break;
                    }
                    buddyTemp = buddyTemp -> next;
                }
                removeElement(&usedHead, &curUsed, temp);
            }
        }
        else{
            temp -> size--;
        }
        temp = next;
    }
}

void t_gcollect(){
    void* stackTop;
    for(void** i = &stackTop; i < (void**) stackBottom; i++){
        if(*i != NULL){
            metadata* temp = usedHead;
            while(temp != NULL){
                if(*i >= temp -> usableMem && *i < temp -> usableMem + temp -> size){
                    if(temp -> size % 4 == 0){
                        temp -> size++;
                        markHeap(temp -> usableMem, temp -> usableMem + temp -> size);
                    }
                    break;
                }
                temp = temp -> next;
            }
        }
    }
    sweep();
}

double get_memory_usage_percentage(){
    return total_memory_allocated;
    //return ((double)memory_in_use / total_memory_allocated) * 100;
}

size_t get_overhead(){
    size_t overhead = 0;
    metadata* temp = freeHead;
    while(temp != NULL){
        printf("found in free: %d\n", temp -> size);
        overhead += HEADER_SIZE;
        temp = temp -> next;
    }
    metadata* temp2 = usedHead;
    while(temp2 != NULL){
        printf("found in used: %d\n", temp2 -> size);
        overhead += HEADER_SIZE;
        temp2 = temp2 -> next;
    }
    return overhead;
}