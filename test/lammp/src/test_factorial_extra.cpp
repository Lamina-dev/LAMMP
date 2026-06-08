/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include <chrono>

#include "../include/test_short.hpp"

#define ALLOC_TYPE(n, type) (type*)lmmp_alloc((n) * sizeof(type))

void test_2factorial() {
    size_t n = 617;
    mp_bitcnt_t bits;
    size_t len1 = lmmp_2factorial_size_(n, &bits);

    size_t len2 = lmmp_arith_seqprod_size_(1, n / 2, 2);
    mp_ptr a = ALLOC_TYPE(len1, mp_limb_t);
    mp_ptr b = ALLOC_TYPE(len2, mp_limb_t);
    
    auto start = std::chrono::high_resolution_clock::now();
    mp_size_t bn = lmmp_arith_seqprod_(b, len2, 1, n / 2, 2);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "Time elapsed: " << duration << " microseconds" << std::endl;

    auto start2 = std::chrono::high_resolution_clock::now();
    mp_size_t an = lmmp_2factorial_(a, bits, len1, n);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();
    std::cout << "Time elapsed: " << duration2 << " microseconds" << std::endl;
    std::cout << std::hex << a[an - 1] << std::endl;
    if (an != bn) {
        std::cout << "an != bn" << std::endl;
        std::cout << "an = " << an << " bn = " << bn << std::endl;
        goto fail;
    }

    for (size_t i = 0; i < an; i++) {
        if (a[i] != b[i]) {
            std::cout << "i = " << i << " a = " << a[i] << " b = " << b[i] << std::endl;
            goto fail;
        }
    }
    std::cout << "passed\n";
fail:
    lmmp_free(a);
    lmmp_free(b);
}

void test_hyperfac() {
    size_t n = 12117;
    mp_bitcnt_t bits;
    size_t len1 = lmmp_hyperfac_size_(n, &bits);

    mp_ptr a = ALLOC_TYPE(len1, mp_limb_t);

    auto start = std::chrono::high_resolution_clock::now();
    mp_size_t an = lmmp_hyperfac_(a, bits, len1, n);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "Time elapsed: " << duration << " microseconds" << std::endl;

    std::cout << "memory used:  " << (an * sizeof(mp_limb_t) / 1024 / 1024) << " Mb" << std::endl;
    std::cout << std::hex << a[an - 1] << " " << a[an - 2] << std::endl;

    lmmp_free(a);
}

void test_superfac() {
    size_t n = 617;
    mp_bitcnt_t bits;
    size_t len1 = lmmp_superfac_size_(n, &bits);

    mp_ptr a = ALLOC_TYPE(len1, mp_limb_t);

    auto start = std::chrono::high_resolution_clock::now();
    mp_size_t an = lmmp_superfac_(a, bits, len1, n);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "Time elapsed: " << duration << " microseconds" << std::endl;

    std::cout << std::hex << a[an - 1] << " " << a[an - 2] << std::endl;

    lmmp_free(a);
}