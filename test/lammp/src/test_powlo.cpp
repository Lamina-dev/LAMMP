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

#include "../include/test_short.hpp"


void test_powlo() {
#define n 3
    mp_limb_t base[n] = {1201, 0, 0xf123323};
#define en 4
    mp_limb_t ep[en] = {10222, 1092013, 0x8aa546cbad0, 0xd23ce233abc23};
    mp_limb_t dst[n];

    printf("base: ");
    for (int i = n - 1; i >= 0; i--) {
        printf("%016llx", base[i]);
    }
    printf("\n");
    printf("ep: ");
    for (int i = en - 1; i >= 0; i--) {
        printf("%016llx", ep[i]);
    }
    printf("\n");

    auto start = std::chrono::high_resolution_clock::now();
    lmmp_powlo_(dst, base, n, ep, en);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "time: " << duration << " microseconds" << std::endl;

    for (int i = n - 1; i >= 0; i--) {
        printf("%016llx", dst[i]);
    }
}