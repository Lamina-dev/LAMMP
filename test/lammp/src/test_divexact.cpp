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
    lmmp_free(p3);
}

void test_divexact_basecase() {
    mp_size_t bn = 31;
    mp_size_t n = 70;
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
    lmmp_divexact_unbalanced_(p2, p1, p1n, b, bn, NULL);
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
    lmmp_free(b);
    lmmp_free(p0);
    lmmp_free(p1);
    lmmp_free(p2);
    lmmp_free(p3);
}

void test_divexact() {
    mp_size_t bn = 17131;
    mp_size_t n = 16220;
    mp_ptr b = ALLOC_TYPE(bn, mp_limb_t);
    lmmp_seed_random_(b, bn, 19123311, 1);
    b[0] |= 1;
    b[bn - 1] |= ((mp_limb_t)1 << (LIMB_BITS - 1));

    mp_ptr p0 = (mp_ptr)lmmp_alloc(n * sizeof(mp_limb_t));
    lmmp_seed_random_(p0, n, 198132271, 2);
    p0[n - 1] |= ((mp_limb_t)1 << (LIMB_BITS - 1));
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
    lmmp_divexact_(p3, p1, p1n, b, bn);
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " us"
              << std::endl;

    for (mp_size_t i = 0; i < p1n - bn; i++) {
        if (p3[i] != p2[i] || p0[i] != p3[i]) {
            printf("Error at index %llu\n", i);
            break;
        }
    }
    if (p3[p1n - bn] != 0 || p2[p1n - bn] != 0) {
        printf("Error at index %llu\n", p1n - bn);
    } 
    lmmp_free(b);
    lmmp_free(p0);
    lmmp_free(p1);
    lmmp_free(p2);
    lmmp_free(p3);
}
