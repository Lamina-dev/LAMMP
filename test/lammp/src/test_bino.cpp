/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include <chrono>
#include "../include/test_short.hpp"

mp_size_t bino_native(mp_ptr dst, mp_size_t rn, ulong n, ulong r) {
    mp_bitcnt_t bits;
    mp_size_t tn = lmmp_nPr_size_(n, r, &bits);
    mp_ptr t = (mp_ptr)lmmp_alloc(tn * sizeof(mp_limb_t));

    tn = lmmp_nPr_(t, bits, tn, n, r);

    mp_size_t prn = lmmp_factorial_size_(r, &bits);
    mp_ptr pr = (mp_ptr)lmmp_alloc(prn * sizeof(mp_limb_t));

    prn = lmmp_factorial_(pr, bits, prn, r);

    lmmp_div_(dst, NULL, t, tn, pr, prn);
    lmmp_free(t);
    lmmp_free(pr);
    rn = tn - prn + 1;
    while (dst[rn - 1] == 0) {
        --rn;
    }
    return rn;
}

#define ALLOC_TYPE(n, type) (type*)lmmp_alloc((n) * sizeof(type))

void test_bino() {
    size_t n = 0x302000, r = 33211;

    mp_bitcnt_t bits = 0;
    size_t len = lmmp_nCr_size_(n, r, &bits);
    std::cout << "len = " << len << std::endl;

    mp_ptr a = ALLOC_TYPE(len, mp_limb_t);
    mp_ptr b = ALLOC_TYPE(len, mp_limb_t);

    a[0] = 1;
    mp_size_t an = 1;
    auto start = std::chrono::high_resolution_clock::now();
    an = bino_native(a, len, n, r);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "Time elapsed: (native)" << duration << " microseconds" << std::endl;

    auto start2 = std::chrono::high_resolution_clock::now();
    mp_size_t bn = lmmp_nCr_(b, bits, len, n, r);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();
    std::cout << "Time elapsed: (queued)" << duration2 << " microseconds" << std::endl;

    if (bn != an) {
        std::cout << "bn != an" << std::endl;
        std::cout << bn << " == " << an << std::endl;
        goto failed;
    }
    for (size_t i = 0; i < an; i++) {
        if (a[i] != b[i]) {
            std::cout << "i = " << i << " a[i] = " << a[i] << " b[i] = " << b[i] << std::endl;
            goto failed;
        }
    }
    std::cout << "passed\n";
    failed:
    lmmp_free(a);
    lmmp_free(b);
}