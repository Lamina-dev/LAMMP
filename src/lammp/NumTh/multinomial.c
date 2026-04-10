/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../../../include/lammp/impl/heap.h"
#include "../../../include/lammp/impl/prime_table.h"
#include "../../../include/lammp/impl/factors_mul.h"

#define LOG2_ 0.693147180559945

mp_size_t lmmp_multinomial_size_(const uintp r, uint m, ulong* restrict n) {
    *n = 0;
    uint i = 0;
    for (; i < m; ++i) *n += r[i];

    double logr = lgamma(*n + 1.0);
    
    for (i = 0; i < m; ++i) 
        logr -= lgamma(r[i] + 1.0);

    logr /= LOG2_;

    mp_size_t rn = ceil(logr / LIMB_BITS) + 2;
    return rn;
}

mp_size_t lmmp_multinomial_(mp_ptr restrict dst, mp_size_t rn, uint n, const uintp restrict r, uint m) {
    lmmp_param_assert(m > 0 && n > 0);
    if (n <= 20) {
        lmmp_nPr_short_(dst, rn, n, n);
        mp_limb_t t = 0;
        for (uint i = 0; i < m; ++i) {
            lmmp_nPr_short_(&t, 1, r[i], r[i]);
            dst[0] /= t;
        }
        return 1;
    }

    lmmp_prime_int_table_init_(n);
    TEMP_B_DECL;
    uint nfactors = lmmp_prime_size_(n);
    factors restrict fac = BALLOC_TYPE(nfactors, factor);
    nfactors = 0;
    for (uint i = 3; i <= n; i += 2) {
        if (!lmmp_is_prime_table_(i))
            continue;
        uint pn = n;
        uint e = 0;
        while (pn > 0) {
            pn /= i;
            e += pn;
        }
        for (uint j = 0; j < m; ++j) {
            pn = r[j];
            while (pn > 0) {
                pn /= i;
                e -= pn;
            }
        }
        if (e > 0) {
            fac[nfactors].f = i;
            fac[nfactors++].j = e;
        }
    }

    mp_size_t shl = n - lmmp_limb_popcnt_(n);
    for (uint j = 0; j < m; ++j) {
        shl += lmmp_limb_popcnt_(r[j]);
        shl -= r[j];
    }
    mp_size_t shw = shl / LIMB_BITS;
    shl %= LIMB_BITS;

    lmmp_zero(dst, shw);
    rn = lmmp_factors_mul_(dst + shw, rn - shw, fac, nfactors, n);

    dst[shw + rn] = lmmp_shl_(dst + shw, dst + shw, rn, shl);
    rn += shw + 1;
    rn -= dst[rn - 1] == 0 ? 1 : 0;

    TEMP_B_FREE;
    return rn;
}
