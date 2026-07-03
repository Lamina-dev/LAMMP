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

long long bench_pow(int len1, int len2) {
    mp_ptr a = ALLOC_TYPE(len1, mp_limb_t);
    ger_random_numbers(len1, a);

    mp_size_t bn = lmmp_pow_size_(a, len1, len2);
    mp_ptr b = ALLOC_TYPE(bn, mp_limb_t);
    
    auto start = std::chrono::high_resolution_clock::now();
    bn = lmmp_pow_(b, bn, a, len1, len2);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    lmmp_free(a);
    lmmp_free(b);
    return duration.count();
}