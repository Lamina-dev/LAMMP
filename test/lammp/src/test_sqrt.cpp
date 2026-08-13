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

#include <chrono>
#include <iostream>

#include "../include/test_short.hpp"


#define ALLOC_TYPE(n, type) (type*)lmmp_alloc((n) * sizeof(type))


void test_sqrt() {
    mp_size_t len = 100, ni = 10000;
    mp_ptr a0 = ALLOC_TYPE(len, mp_limb_t);

    // lmmp_fill(a0, 0, len, 0);
    len = lmmp_seed_random_(a0, len, 2827384783234, 1);
    a0[len - 1] = 1;

    mp_size_t an = ni + len / 2 + 1;
    mp_ptr a = ALLOC_TYPE(an, mp_limb_t);


    //for (mp_ssize_t i = len - 1; i >= 0; i--) {
    //    printf("%016llx", a0[i]);
    //}
    //printf("\n");


    auto start2 = std::chrono::high_resolution_clock::now();
    lmmp_sqrt_(a, NULL, a0, len, ni);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();
    std::cout << duration2 << "\n";

    //for (mp_ssize_t i = an - 1; i >= 0; i--) {
    //    printf("%016llx", a[i]);
    //}

    lmmp_free(a0);
    lmmp_free(a);
}