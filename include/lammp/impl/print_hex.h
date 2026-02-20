#ifndef __LAMMP_PRINT_HEX_H__
#define __LAMMP_PRINT_HEX_H__

#include "../lmmp.h"
#include <stdio.h>

static inline void lmmp_print_hex(mp_srcptr num, mp_ssize_t n) {
    if (n < 0) {
        printf("-");
        n = -n;
    }
    printf("0x");
    for (mp_ssize_t i = n - 1; i >= 0; i--) {
        printf("%llx", num[i]);
    }
    printf("\n");
}

#endif // __LAMMP_PRINT_HEX_H__