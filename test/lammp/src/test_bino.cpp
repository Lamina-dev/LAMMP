/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include <chrono>
#include "../include/test_short.hpp"


mp_size_t lmmp_nPr_odd_(mp_ptr dst, mp_size_t rn, ulong n, ulong r) {
    if (n <= 0xffff) {
        return lmmp_odd_nPr_ushort_(dst, rn, n, r);
    } else {
        return lmmp_odd_nPr_uint_(dst, rn, n, r);
    }
}

mp_size_t bino_native(mp_ptr dst, mp_size_t rn, ulong n, ulong r) {
    mp_bitcnt_t bits1;
    mp_size_t tn = lmmp_nPr_size_(n, r, &bits1);
    mp_ptr tp = (mp_ptr)lmmp_alloc(tn * sizeof(mp_limb_t));
    mp_size_t shw1 = bits1 / LIMB_BITS;
    tn = lmmp_nPr_odd_(tp, tn - shw1, n, r);

    mp_size_t bits2;
    mp_size_t prn = lmmp_factorial_size_(r, &bits2);
    mp_size_t shw2 = bits2 / LIMB_BITS;
    mp_ptr pr = (mp_ptr)lmmp_alloc(prn * sizeof(mp_limb_t));
    prn = lmmp_nPr_odd_(pr, prn - shw2, r, r);

    mp_bitcnt_t bits = bits1 - bits2;
    mp_size_t shw = bits / LIMB_BITS;
    bits = bits % LIMB_BITS;

    lmmp_zero(dst, shw);
    lmmp_debug_assert(rn >= shw + tn - prn + 1);
    //auto start1 = std::chrono::high_resolution_clock::now();
    lmmp_divexact_(dst + shw, tp, tn, pr, prn);
    //auto end1 = std::chrono::high_resolution_clock::now();
    //auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();
    //std::cout << tn << "," << prn << "," << duration1 << ",";

    rn = tn - prn + 1;
    if (bits > 0) {
        dst[shw + rn] = lmmp_shl_(dst + shw, dst + shw, rn, bits);
        rn += shw + 1;
        rn -= dst[rn - 1] == 0;
    } else {
        rn += shw;
    }

    lmmp_free(tp);
    lmmp_free(pr);
    while (dst[rn - 1] == 0) {
        --rn;
    }
    return rn;
}

#define ALLOC_TYPE(n, type) (type*)lmmp_alloc((n) * sizeof(type))

static void test_temp(mp_size_t n, mp_size_t r) {

    mp_bitcnt_t bits = 0;
    size_t len = lmmp_nCr_size_(n, r, &bits);

    mp_ptr a = ALLOC_TYPE(len + 0, mp_limb_t);
    mp_ptr b = ALLOC_TYPE(len, mp_limb_t);

    a[0] = 1;
    mp_size_t an = 1;
    auto start = std::chrono::high_resolution_clock::now();
    an = bino_native(a, len + 0, n, r);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << duration << ",";

    auto start2 = std::chrono::high_resolution_clock::now();
    mp_size_t bn = lmmp_nCr_(b, bits, len, n, r);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();
    std::cout << duration2 << ";";

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
failed:
    lmmp_free(a);
    lmmp_free(b);
}

void test_bino() {
    for (size_t n = 50000; n < 10000000; n += 50000) {
        for (size_t i = 5; i <= 25; i++) {
            mp_size_t r = (n * i) / 2000;
            std::cout << n << "," << r << ",";
            test_temp(n, r);
        }
    }
}