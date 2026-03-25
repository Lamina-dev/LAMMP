/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include <stdio.h>
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
