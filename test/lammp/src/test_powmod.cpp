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

void test_powmod() {
    mp_size_t len = 1000;

    uintp a = (uintp)lmmp_alloc(len * sizeof(uint));
    ulongp b = (ulongp)lmmp_alloc(len * sizeof(ulong));
    uintp c = (uintp)lmmp_alloc(len * sizeof(uint));

    for (mp_size_t i = 0; i < len; i++) {
        c[i] = 2ull * rand() * rand();
        c[i] |= 1;
        a[i] = (1237893246ull * c[i]) % c[i];
        b[i] = (uint64_t)rand() * rand() * 7686812736ull;
    }
    auto start = std::chrono::high_resolution_clock::now();
    for (mp_size_t i = 0; i < len; i++) {
        c[i] = lmmp_powmod_uint_(a[i], b[i], c[i]);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    std::cout << "powmod time: " << duration << " microseconds" << std::endl;
    for (mp_size_t i = 0; i < len; i++) {
        std::cout << c[i] << ",";
    }
    lmmp_free(a);
    lmmp_free(b);
    lmmp_free(c);
}