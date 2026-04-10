/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include <chrono>

#include "../include/test_short.hpp"


#define ALLOC_TYPE(n, type) (type*)lmmp_alloc((n) * sizeof(type))

void test_perm() {
    lmmp_stack_init();
    const size_t n = 0x12203, r = 52222;
    size_t len = lmmp_nPr_size_(n, r);
    std::cout << "len = " << len << std::endl;

    mp_ptr a;
    mp_ptr b = ALLOC_TYPE(len, mp_limb_t);

    mp_limb_t* limb_vec = ALLOC_TYPE(r, mp_limb_t);
    for (size_t i = 1; i <= r; i++) {
        limb_vec[i - 1] = n - r + i;
    }

    auto start = std::chrono::high_resolution_clock::now();
    mp_size_t an = lmmp_limb_elem_mul_(&a, limb_vec, r);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "Time elapsed: " << duration << " microseconds" << std::endl;

    auto start2 = std::chrono::high_resolution_clock::now();
    mp_size_t bn = lmmp_nPr_(b, len, n, r);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();
    std::cout << "Time elapsed: " << duration2 << " microseconds" << std::endl;

    if (bn != an)
        std::cout << "bn != an\n" << bn << " != " << an << std::endl;
    else
        std::cout << bn << " == " << an << std::endl;
    for (size_t i = 0; i < an; i++) {
        if (a[i] != b[i]) {
            std::cout << "i = " << i << " a[i] = " << a[i] << " b[i] = " << b[i] << std::endl;
            goto fail;
        }
    }
    std::cout << "passed\n";
fail:
    lmmp_free(a);
    lmmp_free(b);
    lmmp_free(limb_vec);
}