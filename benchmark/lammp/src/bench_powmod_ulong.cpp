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

long long bench_powmod_ulong(int len) {
    mp_ptr a = (mp_ptr) lmmp_alloc(len * sizeof(mp_limb_t));
    mp_ptr b = (mp_ptr) lmmp_alloc(len * sizeof(mp_limb_t));
    mp_ptr c = (mp_ptr) lmmp_alloc(len * sizeof(mp_limb_t));

    mp_size_t m = 10123221378982007;
    ger_random_numbers(len, a, 0, LIMB_MAX);
    ger_random_numbers(len, b, 0, LIMB_MAX);

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < len; i++) {
        c[i] = lmmp_powmod_ulong_(a[i], b[i], m);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    lmmp_free(a);
    lmmp_free(b);
    lmmp_free(c);
    return duration;
}