/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../../../include/lammp/impl/ele_mul.h"
#include "../../../include/lammp/impl/mparam.h"
#include "../../../include/lammp/impl/prime_table.h"
#include "../../../include/lammp/impl/tmp_alloc.h"
#include "../../../include/lammp/lmmpn.h"
#include "../../../include/lammp/numth.h"


#define MUL(dst, ap, an, bp, bn)        \
    if (an >= bn)                       \
        lmmp_mul_(dst, ap, an, bp, bn); \
    else                                \
        lmmp_mul_(dst, bp, bn, ap, an)

#define mul_1(dst, rn, value)                   \
    dst[rn] = lmmp_mul_1_(dst, dst, rn, value); \
    ++rn;                                       \
    rn -= dst[rn - 1] == 0

mp_size_t lmmp_factorial_size_(uint n, mp_bitcnt_t* restrict bits) {
    double ln_fact = lgamma(n + 1.0);
    double log2_fact = ln_fact / LOG2_;
    mp_size_t rn = ceil(log2_fact / LIMB_BITS) + 2; /* more two limbs */
    *bits = n - lmmp_limb_popcnt_(n);
    return rn;
}

/*
     N                      N/2                              N
    +--+                /  +--+                  \ 2     /  +--+                     \
    |  |  P_i ^ (e_i) = |  |  | P_i ^ (e_i / 2)  |    *  |  |  |  P_i ^ ( e_i mod 2) |  
    |  |                \  |  |                  /       \  |  |                     /
    i=0                    i=0                              i=0
*/

mp_size_t lmmp_factors_mul_(mp_ptr restrict dst, mp_size_t rn, fac_srcptr restrict fac, uint nfactors, uint N) {
    lmmp_param_assert(dst != NULL && fac != NULL);
    lmmp_param_assert(rn > 0 && nfactors > 0 && N > 0);
    if (N <= 0xff || nfactors <= 20) {
        // 对于组合数与多项式系数，某些很不平衡的情况下，N可能很大，所以使用nfactors来进行额外判断。
        // 绝大多数情况下，大的质因数的指数都很小，所以这里只需要考虑小的质因数。
        // 且随着递归的进行，进入这一步的质因数通常都很小。几乎不可能触发debug_assert
        dst[0] = 1;
        rn = 1;
        for (uint i = 0; i < nfactors; i++) {
            ulong f = fac[i].f;
            uint j = fac[i].j;
            lmmp_debug_assert(j != 0);
            switch (j) {
                case 1:
                    mul_1(dst, rn, f);
                    break;
                case 2:
                    lmmp_debug_assert(f <= MP_UINT_MAX);
                    mul_1(dst, rn, f * f);
                    break;
                case 3:
                    lmmp_debug_assert(f <= 2642245);  // 2642245 = floor(B^(1/3))
                    mul_1(dst, rn, f * f * f);
                    break;
                case 4:
                    lmmp_debug_assert(f <= MP_USHORT_MAX);
                    mul_1(dst, rn, f * f * f * f);
                    break;
                default:
                    lmmp_debug_assert(f <= MP_USHORT_MAX);
                    ulong p2 = f * f;
                    ulong p4 = p2 * p2;
                    for (uint i = 0; i < j - 3; i += 4) {
                        mul_1(dst, rn, p4);
                    }
                    switch (j % 4) {
                        case 1:
                            mul_1(dst, rn, f);
                            break;
                        case 2:
                            mul_1(dst, rn, p2);
                            break;
                        case 3:
                            mul_1(dst, rn, p2 * f);
                            break;
                        default:
                            break;
                    }
                    break;
            }
        }
        return rn;
    } else {
        TEMP_DECL;
        // 只有小于 N/2 的质数，其因子的指数才可能大于1，所以新的因子数目不超过 N/2 的素数计数。
        // 同时，由于此估计其实可能严重高估（因为在用于组合数以及多项式系数计算时，此函数的N的衰减远远慢于实际递归缩减），
        // 而考虑到new_nfactors上限一定是nfactors，所以这里比较一次，以尽可能减少分配的空间。
        mp_size_t new_nfactors = lmmp_prime_size_(N / 2);
        new_nfactors = (new_nfactors > nfactors) ? nfactors : new_nfactors;
        fac_ptr restrict new_fac = TALLOC_TYPE(new_nfactors, fac_t);
        new_nfactors = 0;
        ulongp restrict limbs = TALLOC_TYPE(nfactors / 2 + 1, ulong);
        mp_limb_t t = 1;
        mp_size_t limbn = 0;
        for (mp_size_t i = 0; i < nfactors; ++i) {
            lmmp_debug_assert(fac[i].j != 0);
            if (fac[i].j > 1) {
                new_fac[new_nfactors].f = fac[i].f;
                new_fac[new_nfactors++].j = fac[i].j >> 1;
            }
            if (fac[i].j & 1) {
                t *= fac[i].f;
                if (t > MP_UINT_MAX) {
                    limbs[limbn++] = t;
                    t = 1;
                }
            }
        }
        if (t != 1) {
            limbs[limbn++] = t;
        }

        mp_ptr restrict mp = TALLOC_TYPE(limbn * 2, mp_limb_t);
        mp_size_t mpn = 0;

        if (new_nfactors > 0) {
            if (limbn > 0) {
                mpn = lmmp_elem_mul_ulong_(mp, limbs, limbn, mp + limbn);
                lmmp_debug_assert(rn >= mpn);
                mp_size_t tn = ((rn - mpn) >> 1) + 1;
                // 这里根据mpn的大小估计剩余因子乘积的长度，额外分配两倍的tn，以进行平方。
                mp_ptr restrict tp = BALLOC_TYPE(3 * tn + 3, mp_limb_t);
                tn = lmmp_factors_mul_(tp, tn, new_fac, new_nfactors, N / 2);

                mp_ptr restrict tp2 = tp + tn + 1;
                lmmp_sqr_(tp2, tp, tn);
                tn <<= 1;
                tn -= tp2[tn - 1] == 0;
                MUL(dst, tp2, tn, mp, mpn);
                rn = tn + mpn;
                rn -= dst[rn - 1] == 0;
            } else {
                mp_size_t tn = (rn >> 1) + 1;
                mp_ptr restrict tp = TALLOC_TYPE(tn, mp_limb_t);
                tn = lmmp_factors_mul_(tp, tn, new_fac, new_nfactors, N / 2);
                lmmp_sqr_(dst, tp, tn);
                rn = tn << 1;
                rn -= dst[rn - 1] == 0;
            }
        } else {
            lmmp_debug_assert(limbn > 0);
            // 这里不能直接乘入dst，因为dst的大小可能小于limbn，导致溢出
            rn = lmmp_elem_mul_ulong_(mp + limbn, limbs, limbn, mp + limbn);
            lmmp_copy(dst, mp, rn);
        }
        TEMP_FREE;
        return rn;
    }
}

static inline void count_factors(fac_ptr fac, uint nfactors, uint n, uint p) {
    uint pn = n;
    uint e = 0;
    while (pn > 0) {
        pn /= p;
        e += pn;
    }
    fac[nfactors].f = p;
    fac[nfactors].j = e;
}

mp_size_t lmmp_odd_factorial_uint_(mp_ptr restrict dst, mp_size_t rn, uint n) {
    lmmp_prime_int_table_init_(n);
    TEMP_B_DECL;
    uint nfactors = lmmp_prime_size_(n);
    fac_ptr restrict fac = BALLOC_TYPE(nfactors, fac_t);
    nfactors = 0;
    
    prime_cache_t cache;
    lmmp_prime_cache_init_(&cache, n);
    while(cache.is_end == 0) {
        lmmp_prime_cache_next_(&cache);
        for (uint i = 0; i < cache.size; i++) {
            count_factors(fac, nfactors++, n, cache.pp[i]);
        }
    }
    lmmp_prime_cache_free_(&cache);

    rn = lmmp_factors_mul_(dst, rn, fac, nfactors, n);

    TEMP_B_FREE;
    return rn;
}

mp_size_t lmmp_factorial_(mp_ptr restrict dst, mp_bitcnt_t bits, mp_size_t rn, uint n) {
    mp_size_t shw = bits / LIMB_BITS;
    bits %= LIMB_BITS;
    lmmp_zero(dst, shw);
    if (n <= NPR_SHORT_LIMIT)
        rn = lmmp_odd_nPr_ushort_(dst + shw, rn - shw, n, n);
    else
        rn = lmmp_odd_factorial_uint_(dst + shw, rn - shw, n);
    
    if (bits > 0) {
        dst[shw + rn] = lmmp_shl_(dst + shw, dst + shw, rn, bits);
        rn += shw + 1;
        rn -= dst[rn - 1] == 0;
    } else {
        rn += shw;
    }
    return rn;
}