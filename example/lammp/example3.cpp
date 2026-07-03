/**
 *  Copyright (C) 2026 HJimmyK(Jericho Knox)
 *
 *  This file is part of LAMMP.
 *
 *  LAMMP is free software: you can redistribute it and/or modify it under
 *  the terms of the GNU Lesser General Public License (LGPL) as published
 *   by the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed WITHOUT ANY WARRANTY.
 *
 *  See <https://www.gnu.org/licenses/>.
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
