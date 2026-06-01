/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
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
    std::cout << "Time elapsed: (native)" << duration << " microseconds" << std::endl;
    ((void)an);
    lmmp_free(a);
    lmmp_global_deinit();
    lmmp_leak_tracker;
}