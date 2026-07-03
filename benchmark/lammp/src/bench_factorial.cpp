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

#include "../include/benchmark.hpp"

#define ALLOC_TYPE(n, type) (type*)lmmp_alloc((n) * sizeof(type))

long long bench_factorial(unsigned len1) {
    mp_bitcnt_t bits;
    mp_size_t rn = lmmp_factorial_size_(len1, &bits);
    mp_ptr dst = ALLOC_TYPE(rn, mp_limb_t);

    auto start = std::chrono::high_resolution_clock::now();
    rn = lmmp_factorial_(dst, bits, rn, len1);
    auto end = std::chrono::high_resolution_clock::now();

    long long elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    lmmp_free(dst);
    return elapsed_us;
}