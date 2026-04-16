/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include <stdio.h>
#include "../../include/lammp/lmmp.h"
#include "../../include/lammp/mprand.h"

int main() {
    lmmp_global_init();
    lmmp_global_rng_init_(0, 21);
    mp_ptr dst = (mp_ptr)lmmp_alloc(20 * sizeof(mp_limb_t));

    lmmp_random_(dst, 11);

    for (int i = 0; i < 11; i++) {
        printf("%llu ", dst[i]);
    }
    printf("\n");

    lmmp_random_(dst, 10);

    for (int i = 0; i < 10; i++) {
        printf("%llu ", dst[i]);
    }

    lmmp_global_deinit();
    return 0;
}
