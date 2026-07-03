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


void test_arith_seqprod() {
    size_t n = 4824, x = 0x801, m = 0x80;
    size_t len = lmmp_arith_seqprod_size_(x, n, m);
    mp_ptr a = (mp_ptr)lmmp_alloc(len * sizeof(mp_limb_t));

    auto start = std::chrono::high_resolution_clock::now();
    mp_size_t an = lmmp_arith_seqprod_(a, len, x, n, m);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "Time elapsed: " << duration << " microseconds" << std::endl;
    ((void)an);
    lmmp_free(a);
    lmmp_global_deinit();
    lmmp_leak_tracker;
}