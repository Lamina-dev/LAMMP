/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include <chrono>

#include "../include/test_short.hpp"
#include "../../../include/lammp/matrix.h"

void test_arith_seqprod() {
    size_t n = 482404 - 1, x = 160, m = 3;
    size_t len = lmmp_arith_seqprod_size_(x, n, m);
    mp_ptr a = (mp_ptr)lmmp_alloc(len * sizeof(mp_limb_t));
    mp_ptr b;

    auto start = std::chrono::high_resolution_clock::now();
    mp_size_t an = lmmp_arith_seqprod_(a, len, x, n, m);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "Time elapsed: (native)" << duration << " microseconds" << std::endl;

    auto start2 = std::chrono::high_resolution_clock::now();

    mp_limb_t* t = (mp_limb_t*)lmmp_alloc((n + 1) * sizeof(mp_limb_t));
    for (mp_size_t i = 0; i <= n; i++) {
        t[i] = x + i * m;
    }
    mp_size_t bn = lmmp_limb_elem_mul_(&b, t, n + 1);

    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();
    std::cout << "Time elapsed: (native)" << duration2 << " microseconds" << std::endl;

    std::cout << an << " " << bn << std::endl;
    for (size_t i = 0; i < an; i++) {
        if (a[i] != b[i]) {
            std::cout << i << " " << a[i] << " " << b[i] << std::endl;
            std::cout << "failed\n";
            return;
        }
    }

    std::cout << "passed\n";
    lmmp_free(a);
    lmmp_free(b);
    lmmp_free(t);
    lmmp_global_deinit();
    lmmp_leak_tracker;
}