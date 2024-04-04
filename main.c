#include "libtdmm/tdmm.h"
#include <stdio.h>

int main(){
    int x;
    t_init (FIRST_FIT, &x);
    void* ptr1 = t_malloc(109);
    void* ptr2 = t_malloc(9);
    t_free(ptr1);
    void* ptr3 = t_malloc(8);
    void* ptr4 = t_malloc(24);
    t_free(ptr2);
    t_free(ptr3);
    t_free(ptr4);
    //t_gcollect();
    return 0;
}