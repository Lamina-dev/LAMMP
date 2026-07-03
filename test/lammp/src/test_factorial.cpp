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

void test_factorial() {
    size_t n = 9021;
    mp_bitcnt_t bits;
    size_t len = lmmp_factorial_size_(n, &bits);
    mp_ptr a = ALLOC_TYPE(len, mp_limb_t);
    mp_ptr b = ALLOC_TYPE(len, mp_limb_t);

    auto start = std::chrono::high_resolution_clock::now();
    mp_size_t bn = lmmp_nPr_(b, bits, len, n, n);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "Time elapsed: " << duration << " microseconds" << std::endl;
    
    auto start2 = std::chrono::high_resolution_clock::now();
    mp_size_t an = lmmp_factorial_(a, bits, len, n);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();
    std::cout << "Time elapsed: " << duration2 << " microseconds" << std::endl;
    std::cout << std::hex << a[an - 1] << std::endl;
    if (an != bn) {
        std::cout << "an != bn" << std::endl;
        std::cout << "an = " << an << " bn = " << bn << std::endl;
        goto fail;
    }

    for (size_t i = 0; i < an; i++) {
        if (a[i] != b[i]) {
            std::cout << "i = " << i << " a = " << a[i] << " b = " << b[i] << std::endl;
            goto fail;
        }
    }
    std::cout << "passed\n";
    fail:
    lmmp_free(a);
    lmmp_free(b);
}