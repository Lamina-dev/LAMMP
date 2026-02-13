#include <stdio.h>
#include "../../include/lammp/lmmp.h"

int main() {
    mp_size_t size = 100000000000000;
    mp_ptr a = (mp_ptr)lmmp_alloc(size * sizeof(mp_limb_t));
    a[10] = 123456789;
    lmmp_free(a);
    /*
    expected output:
        ...(path): LAMMP abort
        abort type: LAMMP_MEMORY_ALLOC_FAILURE, abort msg:
        Memory allocation failed (size: 800000000000000 bytes)
    */
    return 0;
}
