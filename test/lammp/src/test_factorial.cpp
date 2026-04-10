/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include <chrono>

#include "../include/test_short.hpp"

#define ALLOC_TYPE(n, type) (type*)lmmp_alloc((n) * sizeof(type))

void test_factorial() {
    lmmp_stack_init();

    size_t n = 0xf2233;
    size_t len = lmmp_factorial_size_(n);
    mp_ptr a = ALLOC_TYPE(len, mp_limb_t);
    mp_ptr b = ALLOC_TYPE(len, mp_limb_t);

    auto start = std::chrono::high_resolution_clock::now();
    mp_size_t bn = lmmp_nPr_(b, len, n, n);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "Time elapsed: (native)" << duration << " microseconds" << std::endl;

    auto start2 = std::chrono::high_resolution_clock::now();
    mp_size_t an = lmmp_factorial_int_(a, len, n);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();
    std::cout << "Time elapsed: (queued)" << duration2 << " microseconds" << std::endl;

    std::cout << an << " " << bn << std::endl;
    for (size_t i = 0; i < an; i++) {
        if (a[i] != b[i]) {
            std::cout << "i = " << i << " a = " << a[i] << " b = " << b[i] << std::endl;
            break;
        }
    }
    std::cout << "passed\n";
    lmmp_free(a);
    lmmp_free(b);
    lmmp_global_deinit();
    lmmp_leak_tracker;
}