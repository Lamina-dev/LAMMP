/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include <chrono>

#include "../include/test_short.hpp"


#define ALLOC_TYPE(n, type) (type*)lmmp_alloc((n) * sizeof(type))

void test_perm() {
    mp_bitcnt_t bits;
    ulong n = 1000000, r = 10000;
    size_t len = lmmp_nPr_size_(n, r, &bits);

    mp_ptr b = ALLOC_TYPE(len, mp_limb_t);

    auto start2 = std::chrono::high_resolution_clock::now();
    mp_size_t bn = lmmp_nPr_(b, bits, len, n, r);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();
    std::cout << duration2 << ";";

    ((void)bn);
    lmmp_free(b);
}