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

#define ALLOC_TYPE(n, type) (type*)lmmp_alloc((n) * sizeof(type))

void test_mullo() {
    mp_size_t n = 1938;
    mp_ptr a = ALLOC_TYPE(n, mp_limb_t);
    mp_ptr b = ALLOC_TYPE(n, mp_limb_t);
    mp_ptr c = ALLOC_TYPE(n, mp_limb_t);

    mp_ptr d = ALLOC_TYPE(2 * n, mp_limb_t);

    lmmp_random_(a, n);
    lmmp_random_(b, n);

    auto t1 = std::chrono::high_resolution_clock::now();
    lmmp_mul_n_(d, a, b, n);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    std::cout << "lmmp_mul_n_ time: " << dur << " microseconds" << std::endl;

    t1 = std::chrono::high_resolution_clock::now();
    lmmp_mullo_(c, a, b, n);
    t2 = std::chrono::high_resolution_clock::now();
    dur = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    std::cout << "lmmp_mullo_ time: " << dur << " microseconds" << std::endl;


    for (mp_size_t i = 0; i < n - 1; i++) {
        if (c[i] != d[i]) {
            std::cout << i << "\n";
            std::cout << "c: " << c[i] << " d: " << d[i] << std::endl;
            std::cout << "failed\n";
            break;
        }
    }
    lmmp_free(a);
    lmmp_free(b);
    lmmp_free(c);
    lmmp_free(d);
    lmmp_leak_tracker;
}