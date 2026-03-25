/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../../../include/lammp/impl/heap.h"
#include "../../../include/lammp/impl/prime_table.h"
#include "../../../include/lammp/impl/factors_mul.h"
#include "../../../include/lammp/lmmpn.h"
#include "../../../include/lammp/numth.h"

#define MUL(dst, ap, an, bp, bn)        \
    if (an >= bn)                       \
        lmmp_mul_(dst, ap, an, bp, bn); \
    else                                \
        lmmp_mul_(dst, bp, bn, ap, an)

mp_size_t lmmp_factors_mul_(mp_ptr restrict dst, mp_size_t rn, const factors restrict fac, uint nfactors, uint N) {
    if (N <= 0xff || nfactors <= 20) {
    // 对于某些比较大的N，而因子又不多，递归深度可能不足，所以需要用nfactors来进行额外判断。    
        lmmp_debug_assert(nfactors > 0);
        dst[0] = 1;
        rn = 1;
        for (uint i = 0; i < nfactors; i++) {
            ulong f = fac[i].f;
            lmmp_debug_assert(f < 0xffff);
            if (fac[i].j == 1) {
                dst[rn] = lmmp_mul_1_(dst, dst, rn, f);
                ++rn;
                rn -= dst[rn - 1] == 0;
            } else if (fac[i].j == 2) {
                dst[rn] = lmmp_mul_1_(dst, dst, rn, f * f);
                ++rn;
                rn -= dst[rn - 1] == 0;
            } else if (fac[i].j == 3) {
                dst[rn] = lmmp_mul_1_(dst, dst, rn, f * f * f);
                ++rn;
                rn -= dst[rn - 1] == 0;
            } else if (fac[i].j == 4) {
                f = f * f;
                dst[rn] = lmmp_mul_1_(dst, dst, rn, f * f);
                ++rn;
                rn -= dst[rn - 1] == 0;
            } else {
                ulong p2 = f * f;
                ulong p4 = p2 * p2;
                for (uint j = 0; j < fac[i].j - 3; j += 4) {
                    dst[rn] = lmmp_mul_1_(dst, dst, rn, p4);
                    ++rn;
                    rn -= dst[rn - 1] == 0;
                }
                switch (fac[i].j % 4) {
                    case 1:
                        dst[rn] = lmmp_mul_1_(dst, dst, rn, fac[i].f);
                        ++rn;
                        rn -= dst[rn - 1] == 0;
                        break;
                    case 2:
                        dst[rn] = lmmp_mul_1_(dst, dst, rn, p2);
                        ++rn;
                        rn -= dst[rn - 1] == 0;
                        break;
                    case 3:
                        dst[rn] = lmmp_mul_1_(dst, dst, rn, p2 * fac[i].f);
                        ++rn;
                        rn -= dst[rn - 1] == 0;
                        break;
                    default:
                        break;
                }
            }
        }
        return rn;
    } else {
        TEMP_B_DECL;
        mp_size_t new_nfactors = lmmp_prime_size_(N / 2);
        factors restrict new_fac = BALLOC_TYPE(new_nfactors, factor);
        new_nfactors = 0;

        num_heap heap;
/*
 这是一个比较保守的估计，在阶乘形式下的因子乘积中，可能不需要这么多空间。但这也是为了最坏情况的考虑。
 */
#define heap_size ((nfactors) / FACTORS_MUL_MAX_THRESHOLD)
        lmmp_num_heap_init_(&heap, heap_size + 4);
#undef heap_size

        mp_size_t mpn = 1;
        mp_ptr restrict mp = ALLOC_TYPE(FACTORS_MUL_MAX_THRESHOLD, mp_limb_t);
        mp[0] = 1;
        ulong ulongt = 1;
        for (mp_size_t i = 0; i < nfactors; i++) {
            lmmp_debug_assert(fac[i].j != 0);
            if (fac[i].j > 1) {
                new_fac[new_nfactors].f = fac[i].f;
                new_fac[new_nfactors++].j = fac[i].j >> 1;
            } 
            if (fac[i].j & 1) {
                ulongt *= fac[i].f;
                if (ulongt >= 0xffffffff) {
                    mp[mpn] = lmmp_mul_1_(mp, mp, mpn, ulongt);
                    ulongt = 1;
                    ++mpn;
                    mpn -= mp[mpn - 1] == 0 ? 1 : 0;
                    if (mpn == FACTORS_MUL_MAX_THRESHOLD) {
                        lmmp_num_heap_push_(&heap, mp, mpn);
                        mp = ALLOC_TYPE(FACTORS_MUL_MAX_THRESHOLD, mp_limb_t);
                        mpn = 1;
                        mp[0] = 1;
                    }
                }
            }
        }
        if (ulongt != 1) {
            mp[mpn] = lmmp_mul_1_(mp, mp, mpn, ulongt);
            ulongt = 1;
            ++mpn;
            mpn -= mp[mpn - 1] == 0 ? 1 : 0;
        }
        if (!(mpn == 1 && mp[0] == 1))
            lmmp_num_heap_push_(&heap, mp, mpn);
        else
            lmmp_free(mp);

        lmmp_debug_assert(heap.size != 0);
        mp = lmmp_num_heap_mul_(&heap, &mpn);
        lmmp_num_heap_free_(&heap);

        if (new_nfactors > 0) {
            lmmp_debug_assert(rn >= mpn);
            mp_size_t tn = ((rn - mpn) >> 1) + 1;

            mp_ptr restrict tp = BALLOC_TYPE(3 * tn + 3, mp_limb_t);

            tn = lmmp_factors_mul_(tp, tn, new_fac, new_nfactors, N / 2);

            mp_ptr restrict tp2 = tp + tn + 1;
            lmmp_sqr_(tp2, tp, tn);
            tn <<= 1;
            tn -= tp2[tn - 1] == 0;

            MUL(dst, mp, mpn, tp2, tn);
            rn = tn + mpn;
            rn -= dst[rn - 1] == 0;
        } else {
            lmmp_copy(dst, mp, mpn);
            rn = mpn;
        }
        lmmp_free(mp);
        TEMP_B_FREE;
        return rn;
    }
}

mp_size_t lmmp_factorial_int_(mp_ptr restrict dst, mp_size_t rn, uint n) {
    lmmp_prime_int_table_init_(n);
    TEMP_B_DECL;
    uint nfactors = lmmp_prime_size_(n);
    factors restrict fac = BALLOC_TYPE(nfactors, factor);
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
        fac[nfactors].f = i;
        fac[nfactors++].j = e;
    }

    mp_size_t shl = n - lmmp_limb_popcnt_(n);
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