/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of mylgpllib, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../include/test_short.hpp"
#include <chrono>

void test_mullo() {
    TEMP_B_DECL;
    mp_size_t n = 2238;
    mp_ptr a = BALLOC_TYPE(n, mp_limb_t);
    mp_ptr b = BALLOC_TYPE(n, mp_limb_t);
    mp_ptr c = BALLOC_TYPE(n, mp_limb_t);

    mp_ptr d = BALLOC_TYPE(2 * n, mp_limb_t);

    lmmp_random_(a, n);
    lmmp_random_(b, n);

    auto t1 = std::chrono::high_resolution_clock::now();
    lmmp_mul_n_(d, a, b, n);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    std::cout << "lmmp_mullo_fft_ time: " << dur << " microseconds" << std::endl;

    t1 = std::chrono::high_resolution_clock::now();
    lmmp_mullo_(c, a, b, n);
    t2 = std::chrono::high_resolution_clock::now();
    dur = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    std::cout << "lmmp_mul_n_ time: " << dur << " microseconds" << std::endl;


    for (mp_size_t i = 0; i < n - 1; i++) {
        if (c[i] != d[i]) {
            std::cout << i << "\n";
            std::cout << "c: " << c[i] << " d: " << d[i] << std::endl;
            std::cout << "failed\n";
            break;
        }
    }
    TEMP_B_FREE;
    lmmp_leak_tracker;
}