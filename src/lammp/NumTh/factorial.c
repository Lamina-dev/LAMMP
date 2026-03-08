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

/**
 * @brief 计算因子的累乘，并将结果放入dst中
 * @param dst 结果数组
 * @param rn 结果数组的长度
 * @param fac 因子数组
 * @param nfactors 因子数组的长度
 * @param N 因子的最大值（或最大范围）
 * @warning 因子必须要小于N，且因子必须要单调递增，且不重复，因子的贡献必须要大于0。
 * @return 结果数组的长度
 */
mp_size_t lmmp_factors_mul_(mp_ptr dst, mp_size_t rn, const factors fac, uint nfactors, uint N) {
    if (N <= 0xff) {
        lmmp_debug_assert(nfactors > 0);
        dst[0] = 1;
        rn = 1;
        for (uint i = 0; i < nfactors; i++) {
            ulong f = fac[i].f;
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
        mp_size_t new_fac_cap = lmmp_prime_cnt_table_(N / 2) + 1;
        factors new_fac = BALLOC_TYPE(new_fac_cap, factor);
        mp_size_t new_nfactors = 0;

        num_heap heap;
/*
 这是一个比较保守的估计，在阶乘形式下的因子乘积中，可能不需要这么多空间。但这也是为了最坏情况的考虑。
 */
#define heap_size ((nfactors) / FACTORS_MUL_MAX_THRESHOLD)
        lmmp_num_heap_init_(&heap, heap_size + 4);
#undef heap_size

        mp_size_t mpn = 1;
        mp_ptr mp = ALLOC_TYPE(FACTORS_MUL_MAX_THRESHOLD, mp_limb_t);
        mp[0] = 1;
        ulong ulongt = 1;
        for (mp_size_t i = 0; i < nfactors; i++) {
            lmmp_debug_assert(fac[i].j != 0);
            if (fac[i].j > 1) {
                new_fac[new_nfactors].f = fac[i].f;
                new_fac[new_nfactors++].j = fac[i].j >> 1;
                lmmp_debug_assert(new_nfactors <= new_fac_cap);
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
        mp = lmmp_num_heap_mul_(&heap, &mpn);
        lmmp_num_heap_free_(&heap);

        if (new_nfactors > 0) {
            lmmp_debug_assert(rn >= mpn);
            mp_size_t tn = ((rn - mpn) >> 1) + 1;

            mp_ptr tp = BALLOC_TYPE(3 * tn + 3, mp_limb_t);

            tn = lmmp_factors_mul_(tp, tn, new_fac, new_nfactors, N / 2);

            mp_ptr tp2 = tp + tn + 1;
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

mp_size_t lmmp_factorial_int_(mp_ptr dst, mp_size_t rn, uint n) {
    lmmp_prime_int_table_init_(n, true);
    TEMP_B_DECL;
    uint nfactors = lmmp_prime_cnt_table_(n) - 1;
    factors fac = BALLOC_TYPE(nfactors, factor);
/*
    对于2这个因子，我们单独处理，因为可以通过移位来计算。
 */
    for (uint i = 0; i < nfactors; ++i) {
        fac[i].f = lmmp_nth_prime_table_(i + 1);
        fac[i].j = 0;
        uint pn = n;
        uint e = 0;
        while (pn > 0) {
            pn /= fac[i].f;
            e += pn;
        }
        fac[i].j = e;
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