/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../../../include/lammp/impl/factors_mul.h"
#include "../../../include/lammp/impl/heap.h"
#include "../../../include/lammp/impl/mpdef.h"
#include "../../../include/lammp/impl/prime_table.h"

mp_size_t lmmp_nPr_short_(mp_ptr restrict dst, mp_size_t rn, ulong n, ulong r) {
    lmmp_param_assert(n >= r);
    lmmp_param_assert(n <= MP_USHORT_MAX);
    if (n <= 20) {
        static const ulong factorial[21] = {1,
                                            1,
                                            2,
                                            6,
                                            24,
                                            120,
                                            720,
                                            5040,
                                            40320,
                                            362880,
                                            3628800,
                                            39916800,
                                            479001600,
                                            6227020800,
                                            87178291200,
                                            1307674368000,
                                            20922789888000,
                                            355687428096000,
                                            6402373705728000,
                                            121645100408832000,
                                            2432902008176640000};
        dst[0] = factorial[n] / factorial[n - r];
        return 1;
    } else if (r <= 10) {
        dst[0] = 1;
        rn = 1;
        ulong t = 0;
        ulong i = n - r + 1;
        lmmp_debug_assert(n >= 3);
        for (; i <= (ulong)n - 3; i += 3) {
            t = i * (i + 1) * (i + 2);
            dst[rn] = lmmp_mul_1_(dst, dst, rn, t);
            ++rn;
            rn -= dst[rn - 1] == 0 ? 1 : 0;
        }
        t = 1;
        for (; i <= n; ++i) {
            t *= i;
        }
        dst[rn] = lmmp_mul_1_(dst, dst, rn, t);
        ++rn;
        rn -= dst[rn - 1] == 0 ? 1 : 0;
        return rn;
    } else if (n <= MP_UCHAR_MAX) {
        lmmp_debug_assert(n >= 7);
        lmmp_debug_assert(r >= 2);
        dst[0] = n - r + 1;
        rn = 1;
        ulong t = 0;
        ulong i = n - r + 2;
        for (; i <= (ulong)n - 7; i += 7) {
            t = i * (i + 1) * (i + 2) * (i + 3) * (i + 4) * (i + 5) * (i + 6);
            dst[rn] = lmmp_mul_1_(dst, dst, rn, t);
            ++rn;
            rn -= dst[rn - 1] == 0 ? 1 : 0;
        }
        t = 1;
        for (; i <= n; ++i) {
            t *= i;
        }
        dst[rn] = lmmp_mul_1_(dst, dst, rn, t);
        ++rn;
        rn -= dst[rn - 1] == 0 ? 1 : 0;
        return rn;
    } else if (n <= 0xfff) {
        TEMP_S_DECL;
        ulongp restrict limbs = SALLOC_TYPE(r / 5 + 1, ulong);
        mp_size_t limbn = 0;
        ulong t;
        ulong i = n - r + 1;
        lmmp_debug_assert(n >= 5);
        for (; i <= (ulong)n - 5; i += 5) {
            t = i * (i + 1) * (i + 2) * (i + 3) * (i + 4);
            limbs[limbn++] = t;
        }
        t = 1;
        for (; i <= n; ++i) {
            t *= i;
        }
        if (t != 1)
            limbs[limbn++] = t;
        mp_ptr restrict tp = SALLOC_TYPE(limbn * 2, mp_limb_t);
        rn = lmmp_elem_mul_ulong_(tp, limbs, limbn, tp + limbn);
        lmmp_copy(dst, tp, rn);
        TEMP_S_FREE;
        return rn;
    } else if (4 * n >= 5 * r || rn < PERMUTATION_RN_BASECASE_THRESHOLD) {
        TEMP_DECL;
        ulongp restrict limbs = TALLOC_TYPE(r / 2 + 1, ulong);
        mp_size_t limbn = 0;
        ulong t = n - r + 1;
        uint i = n - r + 2;
        for (; i <= n; ++i) {
            t *= i;
            if (t > MP_UINT_MAX) {
                limbs[limbn++] = t;
                t = 1;
            }
        }
        if (t != 1)
            limbs[limbn++] = t;
        mp_ptr restrict tp = TALLOC_TYPE(limbn * 2, mp_limb_t);
        rn = lmmp_elem_mul_ulong_(tp, limbs, limbn, tp + limbn);
        lmmp_copy(dst, tp, rn);
        TEMP_FREE;
        return rn;
    } else {
        lmmp_debug_assert(n <= MP_USHORT_MAX);
        TEMP_DECL;
        uint nfactors = lmmp_prime_size_(n);
        factors restrict fac = TALLOC_TYPE(nfactors, factor);
        r = n - r;
        /*
            对于2这个因子，我们单独处理，因为可以通过移位来计算。
         */
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
            pn = r;
            while (pn > 0) {
                pn /= i;
                e -= pn;
            }
            if (e > 0) {
                fac[nfactors].f = i;
                fac[nfactors++].j = e;
            }
        }

        mp_size_t shl = n - lmmp_limb_popcnt_(n);
        shl -= r - lmmp_limb_popcnt_(r);
        mp_size_t shw = shl / LIMB_BITS;
        shl %= LIMB_BITS;

        lmmp_zero(dst, shw);
        rn = lmmp_factors_mul_(dst + shw, rn - shw, fac, nfactors, n);

        dst[shw + rn] = lmmp_shl_(dst + shw, dst + shw, rn, shl);
        rn += shw + 1;
        rn -= dst[rn - 1] == 0 ? 1 : 0;

        TEMP_FREE;
        return rn;
    }
}

mp_size_t lmmp_nPr_int_(mp_ptr restrict dst, mp_size_t rn, ulong n, ulong r) {
    lmmp_param_assert(n >= r);
    lmmp_param_assert(n <= MP_UINT_MAX);
    if (r <= 10) {
        dst[0] = 1;
        rn = 1;
        for (ulong i = n - r + 1; i <= n; ++i) {
            dst[rn] = lmmp_mul_1_(dst, dst, rn, i);
            ++rn;
            rn -= dst[rn - 1] == 0 ? 1 : 0;
        }
        return rn;
    } else if (rn < PERMUTATION_RN_MUL_THRESHOLD || n >= (PERMUTATION_NR_TIMES_THRESHOLD * r)) {
        TEMP_DECL;
        ulongp restrict limbs = TALLOC_TYPE(r / 2 + 1, ulong);
        mp_size_t limbn = 0;
        ulong t = n - r + 1;
        uint i = n - r + 2;
        for (; i <= n; ++i) {
            t *= i;
            if (t > MP_UINT_MAX) {
                limbs[limbn++] = t;
                t = 1;
            }
        }
        if (t != 1)
            limbs[limbn++] = t;
        mp_ptr restrict tp = TALLOC_TYPE(limbn * 2, mp_limb_t);
        rn = lmmp_elem_mul_ulong_(tp, limbs, limbn, tp + limbn);
        lmmp_copy(dst, tp, rn);
        TEMP_FREE;
        return rn;
    } else {
        lmmp_debug_assert(n <= MP_UINT_MAX);

        TEMP_B_DECL;
        lmmp_prime_int_table_init_(n);
        uint nfactors = lmmp_prime_size_(n);
        factors restrict fac = BALLOC_TYPE(nfactors, factor);
        r = n - r;
        /*
            对于2这个因子，我们单独处理，因为可以通过移位来计算。
            同时，我们从3开始处理，也是因为可以跳过所有偶数。
         */
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
            pn = r;
            while (pn > 0) {
                pn /= i;
                e -= pn;
            }
            if (e > 0) {
                fac[nfactors].f = i;
                fac[nfactors++].j = e;
            }
        }

        mp_size_t shl = n - lmmp_limb_popcnt_(n);
        shl -= r - lmmp_limb_popcnt_(r);
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
}

mp_size_t lmmp_nPr_long_(mp_ptr restrict dst, mp_size_t rn, ulong n, ulong r) {
    lmmp_param_assert(n >= r);
    TEMP_DECL;
    ulongp restrict limbs = TALLOC_TYPE(r + 1, ulong);
    mp_size_t limbn = r;
    for (ulong i = 1; i <= r; ++i) {
        limbs[i - 1] = n - r + i;
    }
    mp_ptr restrict tp = TALLOC_TYPE(limbn * 2, mp_limb_t);
    rn = lmmp_elem_mul_ulong_(tp, limbs, limbn, tp + limbn);
    lmmp_copy(dst, tp, rn);
    TEMP_FREE;
    return rn;
}
