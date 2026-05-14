/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../include/test_short.hpp"
#include <chrono>

void test_powmod() {
    mp_size_t len = 1000;

    uintp a = (uintp)lmmp_alloc(len * sizeof(uint));
    ulongp b = (ulongp)lmmp_alloc(len * sizeof(ulong));
    uintp c = (uintp)lmmp_alloc(len * sizeof(uint));

    for (mp_size_t i = 0; i < len; i++) {
        c[i] = 2 * rand() * rand();
        a[i] = (rand() * c[i]) % c[i];
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