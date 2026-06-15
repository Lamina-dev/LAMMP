/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include <chrono>

#include "../include/test_short.hpp"


#define ALLOC_TYPE(n, type) (type*)lmmp_alloc((n) * sizeof(type))

void test_divexact_unbalanced() {
    mp_limb_t b = 13422534202027;
    mp_size_t n = 10;
    mp_ptr p0 = (mp_ptr)lmmp_alloc(n * sizeof(mp_limb_t));
    lmmp_seed_random_(p0, n, 1981323762271, 1);
    mp_ptr p1 = (mp_ptr)lmmp_alloc((n + 2) * sizeof(mp_limb_t));

    lmmp_mul_(p1, p0, n, &b, 1);
    mp_size_t p1n = n + 1;
    p1n -= p1[p1n - 1] == 0;

    size_t iter = 100000;
    mp_ptr p2 = (mp_ptr)lmmp_alloc(p1n * sizeof(mp_limb_t));
    mp_ptr p3 = (mp_ptr)lmmp_alloc(p1n * sizeof(mp_limb_t));

    auto start = std::chrono::high_resolution_clock::now();
    mp_limb_t binv = lmmp_binvert_ulong_(b);
    for (mp_size_t i = 0; i < iter; i++) {
        lmmp_divexact_1_(p2, p1, p1n, b, binv);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "divexact_1_ Time taken: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " us"
              << std::endl;

    start = std::chrono::high_resolution_clock::now();
    for (mp_size_t i = 0; i < iter; i++) {
        lmmp_div_1_(p3, p1, p1n, b);
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "div_1_ Time taken:      " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " us"
              << std::endl;

    for (mp_size_t i = 0; i < p1n - 1; i++) {
        if (p3[i] != p2[i]) {
            printf("Error at index %llu\n", i);
            break;
        }
    }
    // for (mp_size_t i = 0; i < n; i++) {
    //     printf("%llu,%llu\n", p1[i], p2[i]);
    // }

    lmmp_free(p0);
    lmmp_free(p1);
    lmmp_free(p2);
}

void test_divexact_basecase() {
    mp_size_t bn = 242;
    mp_size_t n = 50;
    mp_ptr b = ALLOC_TYPE(bn, mp_limb_t);
    lmmp_seed_random_(b, bn, 191278311, 1);
    b[0] |= 1;

    mp_limb_t binv = lmmp_binvert_ulong_(b[0]);
    mp_ptr p0 = (mp_ptr)lmmp_alloc(n * sizeof(mp_limb_t));
    lmmp_seed_random_(p0, n, 198132271, 2);
    mp_ptr p1 = (mp_ptr)lmmp_alloc((n + bn) * sizeof(mp_limb_t));

    if (bn >= n)
        lmmp_mul_(p1, b, bn, p0, n);
    else
        lmmp_mul_(p1, p0, n, b, bn);

    mp_size_t p1n = n + bn;
    p1n -= p1[p1n - 1] == 0;

    mp_ptr p2 = (mp_ptr)lmmp_alloc((p1n - bn + 1) * sizeof(mp_limb_t));
    auto start = std::chrono::high_resolution_clock::now();
    lmmp_divexact_divide_(p2, p1, p1n, b, bn);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " us"
              << std::endl;

    mp_ptr p3 = (mp_ptr)lmmp_alloc((p1n - bn + 1) * sizeof(mp_limb_t));
    start = std::chrono::high_resolution_clock::now();
    lmmp_divexact_basecase_(p3, p1, p1n, b, bn, binv);
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " us"
              << std::endl;

    for (mp_size_t i = 0; i < p1n - bn + 1; i++) {
        if (p3[i] != p2[i]) {
            printf("Error at index %llu\n", i);
            break;
        }
    }
    printf("=================\n%llu,%llu\n", p2[p1n - bn + 1 - 1], p3[p1n - bn + 1 - 1]);
    lmmp_free(b);
    lmmp_free(p0);
    lmmp_free(p1);
    lmmp_free(p2);
    lmmp_free(p3);
}
