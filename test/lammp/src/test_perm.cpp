/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include <chrono>

#include "../include/test_short.hpp"


#define ALLOC_TYPE(n, type) (type*)lmmp_alloc((n) * sizeof(type))

static void temp(size_t n, size_t r) {
    mp_bitcnt_t bits;
    size_t len = lmmp_nPr_size_(n, r, &bits);

    mp_ptr a;
    mp_ptr b = ALLOC_TYPE(len, mp_limb_t);

    mp_limb_t* limb_vec = ALLOC_TYPE(r, mp_limb_t);
    for (size_t i = 1; i <= r; i++) {
        limb_vec[i - 1] = n - r + i;
    }
    auto start1 = std::chrono::high_resolution_clock::now();
    mp_size_t an = lmmp_limb_elem_mul_(&a, limb_vec, r);
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();
    std::cout << duration1 << ",";

    auto start2 = std::chrono::high_resolution_clock::now();
    mp_size_t bn = lmmp_nPr_(b, bits, len, n, r);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();
    std::cout << duration2 << ";";

    if (bn != an)
        std::cout << "bn != an\n" << bn << " != " << an << std::endl;
    for (size_t i = 0; i < an; i++) {
        if (a[i] != b[i]) {
            std::cout << "i = " << i << " a[i] = " << a[i] << " b[i] = " << b[i] << std::endl;
            goto fail;
        }
    }
fail:
    lmmp_free(a);
    lmmp_free(b);
    lmmp_free(limb_vec);
}

void test_perm() {
    // for (size_t i = 1; i <= 200; i++) {
    //     size_t n = 300 * i + 0xfff;
    //     for (size_t j = 1; j <= 30; j++) {
    //         size_t r = j * n / 150;
    //         std::cout << n << "," << r << ",";
    //         temp(n, r);
    //     }
    // }
    temp(100000, 54522);
}