#include "libtdmm/tdmm.h"
#include <stdio.h>

int main(){
    int x;
    t_init (FIRST_FIT, &x);
    void* ptr1 = t_malloc(20000);
    t_free(ptr1);
    //t_gcollect();
    return 0;
}