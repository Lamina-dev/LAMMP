/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include <stdio.h>
#include "../../include/lammp/numth.h"

int main() {
    lmmp_global_init();

    uint n = 10000;
    printf("calculating factorial of %d\n", n);
    mp_bitcnt_t bits;
    mp_size_t len = lmmp_factorial_size_(n, &bits);
    mp_ptr dst = (mp_ptr)lmmp_alloc(len * sizeof(mp_limb_t));
    len = lmmp_factorial_(dst, bits, len, n);
    printf("completed.\n");

    printf("result: %llx ... %llx\n", dst[len - 1], dst[0]);

    lmmp_free(dst);
    lmmp_global_deinit();
    return 0;
}
