#include "libtdmm/tdmm.h"
#include <stdio.h>

int main(){
    int x;
    t_init (WORST_FIT, &x);
    void* ptr1 = t_malloc(199);
    t_free(ptr1);
    t_gcollect();
    return 0;
}