#include "libtdmm/tdmm.h"
#include <stdio.h>

int main(){
    int x;
    t_init (FIRST_FIT, &x);
    void* ptr1 = t_malloc(4096);

    t_free(ptr1);

    void* ptr2 = t_malloc(105);
    void* ptr3 = t_malloc(104);
    void* ptr4 = t_malloc(154);
    void* ptr5 = t_malloc(54);
    void* ptr6 = t_malloc(14);
    void* ptr7 = t_malloc(4);
    void* ptr8 = t_malloc(5);
    void* ptr9 = t_malloc(12);
    void* ptr10 = t_malloc(214);
    void* ptr11 = t_malloc(10544);
    void* ptr12 = t_malloc(1244);
    void* ptr13 = t_malloc(3245);
    void* ptr14 = t_malloc(15);
    void* ptr15 = t_malloc(111);
    void* ptr16 = t_malloc(155);
    void* ptr17 = t_malloc(1056);
    void* ptr18 = t_malloc(1074);
    void* ptr19 = t_malloc(12155);

    
    t_free(ptr2);
    t_free(ptr3);
    t_free(ptr4);
    t_free(ptr5);
    t_free(ptr6);
    t_free(ptr7);
    t_free(ptr8);
    t_free(ptr9);
    t_free(ptr10);
    t_free(ptr11);
    t_free(ptr12);
    t_free(ptr13);
    t_free(ptr14);
    t_free(ptr15);
    t_free(ptr16);
    t_free(ptr17);
    t_free(ptr18);
    t_free(ptr19);
    //t_gcollect();
    return 0;
}