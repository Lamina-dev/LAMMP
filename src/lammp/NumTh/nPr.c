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
        dst[0] = n - r + 1;
        rn = 1;
        ulong t = 0;
        ulong i = n - r + 2;
        lmmp_debug_assert(n >= 7);
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
    } else if (n <= 0xfff && rn < PERMUTATION_RN_BASECASE_THRESHOLD) {
        dst[0] = n - r + 1;
        rn = 1;
        ulong t = 0;
        ulong i = n - r + 2;
        lmmp_debug_assert(n >= 5);
        for (; i <= (ulong)n - 5; i += 5) {
            t = i * (i + 1) * (i + 2) * (i + 3) * (i + 4);
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
        for (uint i = 3; i <= n; ++i) {
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
            fac[nfactors].f = i;
            fac[nfactors++].j = e;
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
    if (rn < PERMUTATION_RN_BASECASE_THRESHOLD) {
        if (r <= 10 || n >= 0x10000000) {
            dst[0] = 1;
            rn = 1;
            for (ulong i = n - r + 1; i <= n; ++i) {
                dst[rn] = lmmp_mul_1_(dst, dst, rn, i);
                ++rn;
                rn -= dst[rn - 1] == 0 ? 1 : 0;
            }
            return rn;
        } else {
            dst[0] = n - r + 1;
            rn = 1;
            ulong t = 0;
            ulong i = n - r + 2;
            for (; i <= (ulong)n - 2; i += 2) {
                t = i * (i + 1);
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
        }
    } else if (rn < PERMUTATION_RN_MUL_THRESHOLD || n >= (PERMUTATION_NR_TIMES_THRESHOLD * r)) {
        num_heap heap;
#define heap_size (r / PERMUTATION_MUL_MAX_THRESHOLD)
        lmmp_num_heap_init_(&heap, LMMP_MAX(heap_size, 4));
#undef heap_size

        mp_size_t mpn = 1;
        mp_ptr restrict mp = ALLOC_TYPE(PERMUTATION_MUL_MAX_THRESHOLD, mp_limb_t);
        mp[0] = 1;
        for (ulong i = n - r + 1; i <= n; ++i) {
            mp[mpn] = lmmp_mul_1_(mp, mp, mpn, i);
            ++mpn;
            mpn -= mp[mpn - 1] == 0 ? 1 : 0;
            if (mpn == PERMUTATION_MUL_MAX_THRESHOLD) {
                lmmp_num_heap_push_(&heap, mp, mpn);
                mp = ALLOC_TYPE(PERMUTATION_MUL_MAX_THRESHOLD, mp_limb_t);
                mpn = 1;
                mp[0] = 1;
            }
        }
        if (!(mpn == 1 && mp[0] == 1))
            lmmp_num_heap_push_(&heap, mp, mpn);
        else
            lmmp_free(mp);

        mp = lmmp_num_heap_mul_(&heap, &mpn);

        lmmp_num_heap_free_(&heap);
        lmmp_copy(dst, mp, mpn);
        
        lmmp_free(mp);

        return mpn;
    } else {
        lmmp_debug_assert(n <= MP_UINT_MAX);

        TEMP_B_DECL;
        lmmp_prime_int_table_init_(n);
        uint nfactors = lmmp_prime_size_(n);
        factors restrict fac = BALLOC_TYPE(nfactors, factor);
        r = n - r;
        /*
            对于2这个因子，我们单独处理，因为可以通过移位来计算。
         */
        nfactors = 0;
        for (uint i = 3; i <= n; ++i) {
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
            fac[nfactors].f = i;
            fac[nfactors++].j = e;
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
    if (rn < PERMUTATION_RN_BASECASE_THRESHOLD) {
        if (n == MP_ULONG_MAX || r <= 3) {
            dst[0] = 1;
            rn = 1;
            for (ulong i = n - r + 1; i != 0 && i <= n; ++i) {
                dst[rn] = lmmp_mul_1_(dst, dst, rn, i);
                ++rn;
                rn -= dst[rn - 1] == 0 ? 1 : 0;
            }
            return rn;
        } else {
            dst[0] = n - r + 1;
            rn = 1;
            for (ulong i = n - r + 2; i <= n; ++i) {
                dst[rn] = lmmp_mul_1_(dst, dst, rn, i);
                ++rn;
                rn -= dst[rn - 1] == 0 ? 1 : 0;
            }
            return rn;
        }
    } else {
        num_heap heap;
#define heap_size (r / PERMUTATION_MUL_MAX_THRESHOLD)
        lmmp_num_heap_init_(&heap, LMMP_MAX(heap_size, 4));
#undef heap_size

        mp_size_t mpn = 1;
        mp_ptr restrict mp = ALLOC_TYPE(PERMUTATION_MUL_MAX_THRESHOLD, mp_limb_t);
        mp[0] = 1;
        for (ulong i = n - r + 1; i <= n; ++i) {
            mp[mpn] = lmmp_mul_1_(mp, mp, mpn, i);
            ++mpn;
            mpn -= mp[mpn - 1] == 0 ? 1 : 0;
            if (mpn == PERMUTATION_MUL_MAX_THRESHOLD) {
                lmmp_num_heap_push_(&heap, mp, mpn);
                mp = ALLOC_TYPE(PERMUTATION_MUL_MAX_THRESHOLD, mp_limb_t);
                mpn = 1;
                mp[0] = 1;
            }
        }

        if (!(mpn == 1 && mp[0] == 1))
            lmmp_num_heap_push_(&heap, mp, mpn);
        else
            lmmp_free(mp);

        mp = lmmp_num_heap_mul_(&heap, &mpn);

        lmmp_copy(dst, mp, mpn);
        lmmp_free(mp);
        lmmp_num_heap_free_(&heap);
        return mpn;
    }
}
