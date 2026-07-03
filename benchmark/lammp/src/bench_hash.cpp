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
#include "../../../include/lammp/secret.h"

long long bench_hash(int len, int type) {
    type %= 2;
    mp_ptr a = (mp_ptr)lmmp_alloc(len * sizeof(mp_limb_t));
    ger_random_numbers(len, a);
    if (type == 0) {
        key128_t key = {1012321, 233320};
        auto start = std::chrono::high_resolution_clock::now();
        uint64_t h = lmmp_siphash24_(a, len, key);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        (void(h));
        lmmp_free(a);
        return duration.count();
    } else {
        key64_t key = {1012321};
        auto start = std::chrono::high_resolution_clock::now();
        uint64_t h = lmmp_xxhash_(a, len, key);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        (void(h));
        lmmp_free(a);
        return duration.count();
    }
}