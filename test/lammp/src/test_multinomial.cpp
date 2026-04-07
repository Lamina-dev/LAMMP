/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include <chrono>
#include "../include/test_short.hpp"

#define ALLOC_TYPE(n, type) (type*)lmmp_alloc((n) * sizeof(type))

void test_multinomial() {
    lmmp_stack_init();
    size_t m = 22;
    uint r[22] = {0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    ulong n = 0;
    size_t len = lmmp_multinomial_size_(r, m, &n);
    std::cout << "len = " << len << std::endl;
    std::cout << "n = " << n << std::endl;

    mp_ptr a = ALLOC_TYPE(len, mp_limb_t);

    auto start2 = std::chrono::high_resolution_clock::now();
    mp_size_t bn = lmmp_multinomial_(a, len, n, r, m);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();
    std::cout << "Time elapsed: (queued)" << duration2 << " microseconds" << std::endl;

    std::cout << "bn = " << bn << std::endl;

    lmmp_free(a);
}