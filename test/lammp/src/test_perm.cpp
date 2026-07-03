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

mp_size_t lmmp_elem_mul_ulong_(mp_ptr dst, const ulongp limbs, mp_size_t n, mp_ptr tp) {
    if (n < 20) {
        lmmp_debug_assert(n > 0);
        dst[0] = limbs[0];
        mp_size_t rn = 1;
        for (mp_size_t i = 1; i < n; i++) {
            dst[rn] = lmmp_mul_1_(dst, dst, rn, limbs[i]);
            rn++;
            rn -= dst[rn - 1] == 0 ? 1 : 0;
        }
        return rn;
    }
    mp_size_t halfn = n / 2;
    mp_size_t n1 = lmmp_elem_mul_ulong_(tp, limbs, halfn, dst);
    mp_size_t n2 = lmmp_elem_mul_ulong_(tp + halfn, limbs + halfn, n - halfn, dst + halfn);
    if (n1 > n2)
        lmmp_mul_(dst, tp, n1, tp + halfn, n2);
    else
        lmmp_mul_(dst, tp + halfn, n2, tp, n1);
    n = n1 + n2;
    n -= dst[n - 1] == 0 ? 1 : 0;
    return n;
}

static void test_tmp(ulong n, ulong r) {
    mp_bitcnt_t bits;
    size_t len = lmmp_nPr_size_(n, r, &bits);

    mp_ptr b = ALLOC_TYPE(len, mp_limb_t);
    mp_ptr c = ALLOC_TYPE(r * 2, mp_limb_t);
    ulongp limbs = ALLOC_TYPE(r, ulong);
    for (ulong i = 1; i <= r; i++) limbs[i - 1] = n - i + 1;

    auto start1 = std::chrono::high_resolution_clock::now();
    mp_size_t cn = lmmp_elem_mul_ulong_(c, limbs, r, c + r);
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();
    std::cout << duration1 << ",";

    auto start2 = std::chrono::high_resolution_clock::now();
    mp_size_t bn = lmmp_nPr_(b, bits, len, n, r);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();
    std::cout << duration2 << ";";

    if (cn != bn) {
        std::cout << "Error: " << cn << " != " << bn << std::endl;
        goto end;
    }

    for (mp_size_t i = 0; i < cn; i++) {
        if (b[i] != c[i]) {
            std::cout << "Error: " << b[i] << " != " << c[i] << std::endl;
            goto end;
        }
    }

end:
    lmmp_free(b);
    lmmp_free(c);
    lmmp_free(limbs);
}

void test_perm() {
    for (size_t n = 100000; n < 6500000; n += 20000) {
        for (size_t i = 1; i <= 50; i += 3) {
            mp_size_t r = (n * i) / 2000;
            std::cout << n << "," << r << ",";
            test_tmp(n, r);
        }
    }
}