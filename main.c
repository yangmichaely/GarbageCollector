#include "libtdmm/tdmm.h"
#include <stdio.h>

int main(){
    int x;
    t_init (FIRST_FIT, &x);
    void* ptr = t_malloc(10);
    t_free(ptr);
    //t_gcollect();
    return 0;
}