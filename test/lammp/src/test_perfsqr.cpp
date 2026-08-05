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

void test_perfsqr_filter() {

    mp_size_t len = 1000000;
    mp_size_t t = 0;
    mp_ptr b = ALLOC_TYPE(len, mp_limb_t);
    lmmp_seed_random_(b, len, 8230232, 1);
    auto start1 = std::chrono::high_resolution_clock::now();
    for (mp_size_t i = 0; i < len; i++) {
        b[i] %= 1ULL << 32;
        b[i] *= b[i] + 1;
        t += lmmp_perfsqr_filter_1_(b[i]);
    }
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();
    std::cout << duration1 << ";";
    std::cout << t << ";";

    lmmp_free(b);
}

void test_perfsqr() {
    mp_size_t len = 1000;
    mp_ptr b = ALLOC_TYPE(len, mp_limb_t);
    mp_ptr d = ALLOC_TYPE(len * 2, mp_limb_t);
    lmmp_seed_random_(b, len, 8230232, 1);
    b[len - 1] |= 1ULL << 32;
    auto start1 = std::chrono::high_resolution_clock::now();
    lmmp_sqr_(d, b, len);
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();
    std::cout << duration1 << "\n";
    lmmp_inc(d);

    auto start2 = std::chrono::high_resolution_clock::now();
    bool is_perfsqr = lmmp_perfsqr_(d, 2 * len);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();
    std::cout << duration2 << "\n";

    if (is_perfsqr) {
        std::cout << "perfect square;\n";
    } else {
        std::cout << "not a perfect square;\n";
    }

    lmmp_free(b);
    lmmp_free(d);
}