/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../../../include/lammp/impl/factors_mul.h"
#include "../../../include/lammp/impl/ele_mul.h"
#include "../../../include/lammp/impl/mparam.h"
#include "../../../include/lammp/impl/prime_table.h"
#include "../../../include/lammp/impl/longlong.h"

static const ulong factorial[21] = {1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880, 3628800,
                                    39916800, 479001600, 6227020800, 87178291200,
                                    1307674368000, 20922789888000, 355687428096000,
                                    6402373705728000, 121645100408832000, 2432902008176640000};

static inline uint count_factors(factors fac, uint nfactors, uint n, uint r, uint p) {
    uint pn = n;
    uint e = 0;
    while (pn > 0) {
        pn /= p;
        e += pn;
    }
    pn = r;
    while (pn > 0) {
        pn /= p;
        e -= pn;
    }
    if (e > 0) {
        fac[nfactors].f = p;
        fac[nfactors++].j = e;
    }
    return nfactors;
}

/**
 * @brief 使用累乘函数计算nPr
 */
static mp_size_t lmmp_nPr_product_(mp_ptr restrict dst, mp_size_t rn, uint n, uint r) {
    TEMP_DECL;
    ulongp restrict limbs = TALLOC_TYPE(r / 2 + 1, ulong);
    mp_size_t limbn = 0;
    ulong t = 1, v;
    mp_bitcnt_t shl = 0, cnt;
    for (ulong i = n - r + 1; i <= n; ++i) {
        ctz_shl(v, i, cnt);
        t *= v;
        shl += cnt;
        if (t > MP_UINT_MAX) {
            limbs[limbn++] = t;
            t = 1;
        }
    }
    if (t != 1)
        limbs[limbn++] = t;

    mp_size_t shw = shl / LIMB_BITS;
    shl %= LIMB_BITS;
    lmmp_zero(dst, shw);

    mp_ptr restrict tp = TALLOC_TYPE(limbn * 2, mp_limb_t);
    // 这里不能直接乘入dst，因为dst的大小可能小于limbn，导致溢出
    rn = lmmp_elem_mul_ulong_(tp, limbs, limbn, tp + limbn);
    dst[shw + rn] = lmmp_shl_(dst + shw, tp, rn, shl);
    rn += shw + 1;
    rn -= dst[rn - 1] == 0 ? 1 : 0;
    TEMP_FREE;
    return rn;
}

mp_size_t lmmp_nPr_short_(mp_ptr restrict dst, mp_size_t rn, ulong n, ulong r) {
    lmmp_param_assert(n >= r);
    lmmp_param_assert(n <= MP_USHORT_MAX);
    if (n <= 20) {
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
    } else if (n <= 0xfff && rn <= PERMUTATION_USHORT_BASECASE_THRESHOLD) {
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
    } else if (rn < PERMUTATION_USHORT_MUL_THRESHOLD || n >= (PERMUTATION_USHORT_TIMES_THRESHOLD * r)) {
        return lmmp_nPr_product_(dst, rn, n, r);
    } else {
        lmmp_debug_assert(n <= MP_USHORT_MAX);
        TEMP_DECL;
        uint primen = lmmp_prime_cnt16_(n);
        uint nfactors = primen;
        factors restrict fac = TALLOC_TYPE(nfactors, factor);
        r = n - r;
        nfactors = 0;
        for (uint i = 1; i < primen ; ++i) {
            uint p = prime_short_table[i];
            nfactors = count_factors(fac, nfactors, n, r, p);
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
    } else if (rn < PERMUTATION_UINT_MUL_THRESHOLD || n >= (PERMUTATION_UINT_TIMES_THRESHOLD * r)) {
        return lmmp_nPr_product_(dst, rn, n, r);
    } else {
        lmmp_debug_assert(n <= MP_UINT_MAX);

        TEMP_B_DECL;
        lmmp_prime_int_table_init_(n);
        uint nfactors = lmmp_prime_size_(n);
        factors restrict fac = BALLOC_TYPE(nfactors, factor);
        r = n - r;
        nfactors = 0;

        prime_cache_t cache;
        lmmp_prime_cache_init_(&cache);
        while(cache.is_end == 0) {
            lmmp_prime_cache_next_(&cache);
            for (uint i = 0; i < cache.size; ++i) {
                nfactors = count_factors(fac, nfactors, n, r, cache.pp[i]);
            }
        }
        lmmp_prime_cache_free_(&cache);

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
    mp_size_t limbn = 0;
    ulong t, v, m = 1;
    mp_bitcnt_t shl = 0, cnt;
    for (ulong i = 1; i <= r; ++i) {
        t = n - r + i;
        ctz_shl(v, t, cnt);
        if (v != 1)
            limbs[limbn++] = v;
        shl += cnt;
    }
    if (m != 1)
        limbs[limbn++] = m;
    
    mp_size_t shw = shl / LIMB_BITS;
    shl %= LIMB_BITS;
    lmmp_zero(dst, shw);

    mp_ptr restrict tp = TALLOC_TYPE(limbn * 2, mp_limb_t);
    // 这里不能直接乘入dst，因为dst的大小可能小于limbn，导致溢出
    rn = lmmp_elem_mul_ulong_(tp, limbs, limbn, tp + limbn);
    dst[shw + rn] = lmmp_shl_(dst + shw, tp, rn, shl);
    rn += shw + 1;
    rn -= dst[rn - 1] == 0 ? 1 : 0;
    TEMP_FREE;
    return rn;
}
