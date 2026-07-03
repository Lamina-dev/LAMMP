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

long long bench_sub(int len1, int len2) {
    if (len1 < len2) {
        std::swap(len1, len2);
    } else if (len1 == len2) {
        len1++;
    }
    mp_ptr a = (mp_ptr)lmmp_alloc(len1 * sizeof(mp_limb_t));
    mp_ptr b = (mp_ptr)lmmp_alloc(len2 * sizeof(mp_limb_t));
    mp_ptr c = (mp_ptr)lmmp_alloc(len1 * sizeof(mp_limb_t));
    auto start = std::chrono::high_resolution_clock::now();
    lmmp_sub_(c, a, len1, b, len2);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    lmmp_free(a);
    lmmp_free(b);
    lmmp_free(c);
    return duration.count();
}
