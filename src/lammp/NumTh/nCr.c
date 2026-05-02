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
#include "../../../include/lammp/numth.h"

mp_size_t lmmp_nCr_size_(uint n, uint r, mp_bitcnt_t* restrict bits) {
    double ln_comb = lgamma(n + 1.0) - lgamma(r + 1.0) - lgamma(n - r + 1.0);
    double log2_comb = ln_comb / LOG2_;
    mp_size_t rn = ceil(log2_comb / LIMB_BITS) + 2; /* more two limbs */
    (*bits) = n - lmmp_limb_popcnt_(n);
    (*bits) -= r - lmmp_limb_popcnt_(r);
    (*bits) -= n - r - lmmp_limb_popcnt_(n - r);
    return rn;
}

static inline uint factor_size_int(mp_size_t rn, uint n) {
    /*
     我们可以知道，nCr的大小一定不会超过B^rn，因此，B^rn的可以含有的质因数数量即为nCr可以含有的质因数数量的上限。
     同时，我们这里只计算的是奇数部分，我们可以用B^rn可以含有的3的质因数个数来估计nCr的质因数种类数。
     这是一个绝对上界，同时在不平衡时比pi(n)这个非平方上界要紧得多。当然即使是这个上界，实际的质因数个数也可能远远
     小于这个上界。一个改进想法是，我们使用更大一点的质数，对于n>0xffff，我们选取这个质数为251
     而log(B)/log(251)约等于8.02855802854906，我们近似视为8，这也是这里的常数的由来。
     我们这里对两个估计进行比较，取较小的一个作为最终结果。不平衡时，approx1要更紧一些。
    */
    uint approx1 = rn * 8;
    uint approx2 = lmmp_prime_size_(n);
    return approx1 < approx2? approx1 : approx2;
}

static inline uint factor_size_short(mp_size_t rn) {
    /*
     其实，经过大量的校验，*8几乎也不会低估，但是为了留有冗余，我们还是选择*10，大致相当于质数83。
    */
    return rn * 10;
}

static inline uint count_factors(factors fac, uint nfactors, uint n, uint r, uint nr, uint p) {
    uint pn = n;
    uint e = 0;
    ulong inv = MP_ULONG_MAX / p + 1;
    while (pn > 0) {
        _udiv32by32_q_preinv(pn, pn, inv);
        e += pn;
    }
    pn = r;
    while (pn > 0) {
        _udiv32by32_q_preinv(pn, pn, inv);
        e -= pn;
    }
    pn = nr;
    while (pn > 0) {
        _udiv32by32_q_preinv(pn, pn, inv);
        e -= pn;
    }
    if (e > 0) {
        fac[nfactors].f = p;
        fac[nfactors++].j = e;
    }
    return nfactors;
}

/*
FIXME:
  当极不平衡时，质因数分解本身的效率很低，因为本身其质因数就极其稀疏。
  因此，当极不平衡时，应该考虑采用其他方法，比如直接计算排列数和阶乘，再使用精确除法。
  当质因数比较稀疏时，排列数可能采用的是累乘法，而阶乘本身就是平衡的。目前精确除法暂未实现。
*/

mp_size_t lmmp_odd_nCr_short_(mp_ptr restrict dst, mp_size_t rn, uint n, uint r) {
    lmmp_param_assert(n <= MP_USHORT_MAX);
    lmmp_param_assert(r <= n / 2);
    if (r < ODD_FACTORIAL_SIZE) {
        rn = lmmp_odd_nPr_short_(dst, rn, n, r);
        mp_limb_t t = 0;
        lmmp_odd_nPr_short_(&t, 1, r, r);
        // FIXME: 这里的除法应该使用精确除法
        lmmp_div_1_(dst, dst, rn, t);
        rn -= dst[rn - 1] == 0 ? 1 : 0;
        return rn;
    } else if (rn < BINOMIAL_RN_BASECASE_THRESHOLD) {
        if (r <= 4 || n > 0xfff) {
            dst[0] = 1;
            rn = 1;
            ulong t, v;
            mp_bitcnt_t cnt;
            for (ulong i = 1; i <= r; ++i) {
                t = n - i + 1;
                ctz_shl(v, t, cnt);
                dst[rn] = lmmp_mul_1_(dst, dst, rn, v);
                ++rn;
                rn -= dst[rn - 1] == 0 ? 1 : 0;
                ctz_shl(v, i, cnt);
                lmmp_div_1_(dst, dst, rn, v);
                rn -= dst[rn - 1] == 0 ? 1 : 0;
            }
            return rn;
        } else {
            dst[0] = 1;
            rn = 1;
            ulong i = 1;
            ulong t = 1, d = 1, v;
            mp_bitcnt_t cnt;
            for (; i <= (ulong)r - 4; i += 4) {
                // 相邻四个数必含有3这个公共因子
                d = i * (i + 1) * (i + 2) * (i + 3);
                d /= 3;
                t = (n - i + 1) * (n - i) * (n - i - 1) * (n - i - 2);
                t /= 3;
                ctz_shl(v, t, cnt);
                dst[rn] = lmmp_mul_1_(dst, dst, rn, v);
                ++rn;
                rn -= dst[rn - 1] == 0 ? 1 : 0;
                ctz_shl(v, d, cnt);
                lmmp_div_1_(dst, dst, rn, v);
                rn -= dst[rn - 1] == 0 ? 1 : 0;
            }
            for (; i <= r; ++i) {
                t = n - i + 1;
                ctz_shl(v, t, cnt);
                dst[rn] = lmmp_mul_1_(dst, dst, rn, v);
                ++rn;
                rn -= dst[rn - 1] == 0 ? 1 : 0;
                ctz_shl(v, i, cnt);
                lmmp_div_1_(dst, dst, rn, v);
                rn -= dst[rn - 1] == 0 ? 1 : 0;
            }
            return rn;
        }
    } else {
        TEMP_DECL;
        uint primen = lmmp_prime_cnt16_(n);
        uint nfactors = factor_size_short(rn);
        nfactors = primen < nfactors ? primen : nfactors;
        factors restrict fac = TALLOC_TYPE(nfactors, factor);
        uint nr = n - r;
        nfactors = 0;
        for (uint i = 1; i < primen; ++i) {
            uint p = prime_short_table[i];
            nfactors = count_factors(fac, nfactors, n, r, nr, p);
        }

        rn = lmmp_factors_mul_(dst, rn, fac, nfactors, n);

        TEMP_FREE;
        return rn;
    }
}

mp_size_t lmmp_odd_nCr_int_(mp_ptr restrict dst, mp_size_t rn, uint n, uint r) {
    lmmp_param_assert(r <= (n / 2));
    if (r <= 3 || (n > 0xfffffff && rn < BINOMIAL_RN_BASECASE_THRESHOLD)) {
        dst[0] = 1;
        rn = 1;
        ulong t, v;
        mp_bitcnt_t cnt;
        for (ulong i = 1; i <= r; ++i) {
            t = n - i + 1;
            ctz_shl(v, t, cnt);
            dst[rn] = lmmp_mul_1_(dst, dst, rn, v);
            ++rn;
            rn -= dst[rn - 1] == 0 ? 1 : 0;
            ctz_shl(v, i, cnt);
            lmmp_div_1_(dst, dst, rn, v);
            rn -= dst[rn - 1] == 0 ? 1 : 0;
        }
        return rn;
    } else if (rn < BINOMIAL_RN_BASECASE_THRESHOLD) {
        dst[0] = 1;
        rn = 1;
        ulong i = 1;
        ulong t = 1, d = 1, v;
        mp_bitcnt_t cnt;
        for (; i <= (ulong)r - 2; i += 2) {
            d = i * (i + 1);
            t = (n - i + 1) * (n - i);
            ctz_shl(v, t, cnt);
            dst[rn] = lmmp_mul_1_(dst, dst, rn, v);
            ++rn;
            rn -= dst[rn - 1] == 0 ? 1 : 0;
            ctz_shl(v, d, cnt);
            lmmp_div_1_(dst, dst, rn, v);
            rn -= dst[rn - 1] == 0 ? 1 : 0;
        }
        for (; i <= r; ++i) {
            t = n - i + 1;
            ctz_shl(v, t, cnt);
            dst[rn] = lmmp_mul_1_(dst, dst, rn, v);
            ++rn;
            rn -= dst[rn - 1] == 0 ? 1 : 0;
            ctz_shl(v, i, cnt);
            lmmp_div_1_(dst, dst, rn, v);
            rn -= dst[rn - 1] == 0 ? 1 : 0;
        }
        return rn;
    } else {
        lmmp_prime_int_table_init_(n);
        TEMP_B_DECL;
        uint nfactors = factor_size_int(rn, n);
        factors restrict fac = BALLOC_TYPE(nfactors, factor);
        uint nr = n - r;

        nfactors = 0;
        prime_cache_t cache;
        lmmp_prime_cache_init_(&cache, n);
        while (cache.is_end == 0) {
            lmmp_prime_cache_next_(&cache);
            for (uint i = 0; i < cache.size; ++i) {
                nfactors = count_factors(fac, nfactors, n, r, nr, cache.pp[i]);
            }
        }
        lmmp_prime_cache_free_(&cache);

        rn = lmmp_factors_mul_(dst, rn, fac, nfactors, n);

        TEMP_B_FREE;
        return rn;
    }
}

mp_size_t lmmp_nCr_(mp_ptr dst, mp_bitcnt_t bits, mp_size_t rn, uint n, uint r) {
    lmmp_debug_assert(r <= (n / 2));
    mp_size_t shw = bits / LIMB_BITS;
    bits %= LIMB_BITS;
    lmmp_zero(dst, shw);
    if (n <= NCR_SHORT_LIMIT)
        rn = lmmp_odd_nCr_short_(dst + shw, rn - shw, n, r);
    else
        rn = lmmp_odd_nCr_int_(dst + shw, rn - shw, n, r);
    if (bits > 0) {
        dst[shw + rn] = lmmp_shl_(dst + shw, dst + shw, rn, bits);
        rn += shw + 1;
        rn -= dst[rn - 1] == 0;
    } else {
        rn += shw;
    }
    return rn;
}