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

#include "../include/test_short.hpp"
#include <chrono>
void test_add_nc() {
    mp_ptr a = (mp_ptr)lmmp_alloc(10 * sizeof(mp_limb_t));
    mp_ptr b = (mp_ptr)lmmp_alloc(10 * sizeof(mp_limb_t));
    a[0] = 1; a[1] = 2; a[2] = 3; a[3] = 4; a[4] = 5; a[5] = 6; a[6] = 7; a[7] = 8; a[8] = 9; a[9] = 10;
    b[0] = 10; b[1] = 20; b[2] = 30; b[3] = 40; b[4] = 50; b[5] = 60; b[6] = 70; b[7] = 80; b[8] = 90; b[9] = 100;
    mp_size_t n = 10;
    mp_limb_t carry = 0;
    auto start = std::chrono::high_resolution_clock::now();
    lmmp_add_nc_(a, a, b, n, carry);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << "elapsed time: " << elapsed.count() << " ms" << std::endl;
    for (int i = 0; i < 10; i++) {
        std::cout << a[i] << " ";
    }
    std::cout << std::endl;
    lmmp_free(a);
    lmmp_free(b);
}

