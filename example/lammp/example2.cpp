#include <stdio.h>
#include "../../include/lammp/lmmp.h"
#include "../../include/lammp/numth.h"

int main() {
    uint n = 100000;
    printf("calculating factorial of %d...\n", n);
    mp_size_t len = lmmp_factorial_size_(n);
    mp_ptr dst = (mp_ptr)lmmp_alloc(len * sizeof(mp_limb_t));
    len = lmmp_factorial_(dst, len, n);
    printf("completed.\n");
    lmmp_free(dst);
    return 0;
}
