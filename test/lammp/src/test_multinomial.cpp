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

#define ALLOC_TYPE(n, type) (type*)lmmp_alloc((n) * sizeof(type))

void test_multinomial() {
    size_t m = 5;
    uint r[5] = {234, 340, 1042, 2034, 23042};
    ulong n = 0;
    size_t len = lmmp_multinomial_size_(r, m, &n);
    std::cout << "len = " << len << std::endl;
    std::cout << "n = " << n << std::endl;

    mp_ptr a = ALLOC_TYPE(len, mp_limb_t);

    auto start2 = std::chrono::high_resolution_clock::now();
    mp_size_t bn = lmmp_multinomial_(a, len, n, r, m);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();
    std::cout << "Time elapsed: " << duration2 << " microseconds" << std::endl;

    std::cout << "bn = " << bn << std::endl;
    std::cout << std::hex << a[bn - 1] << std::endl;
    lmmp_free(a);
}