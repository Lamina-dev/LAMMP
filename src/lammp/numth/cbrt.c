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

#include "../../../include/lammp/impl/inlines.h"
#include "../../../include/lammp/impl/longlong.h"
#include "../../../include/lammp/impl/log2_exp2.h"
#include "../../../include/lammp/impl/tmp_alloc.h"
#include "../../../include/lammp/numth.h"
#include "../../../include/lammp/lmmpn.h"


static inline void lmmp_cube_3_(mp_ptr restrict dst, mp_limb_t a) {
    mp_limb_t t[2];
    lmmp_mullh_(a, a, t);
    lmmp_mullh_(t[0], a, dst);
    lmmp_mullh_(t[1], a, t);
    dst[1] += t[0];
    dst[2] = t[1] + (dst[1] < t[0] ? 1 : 0);
}

static inline mp_size_t lmmp_cube_6_(mp_ptr restrict dst, mp_srcptr restrict numa) {
    mp_limb_t t[4];
    lmmp_sqr_basecase_(t, numa, 2);
    lmmp_mul_basecase_(dst, t, 4, numa, 2);
    mp_size_t n = 6;
    while (dst[n - 1] == 0) --n;
    return n;
}

mp_size_t lmmp_cube_(mp_ptr restrict dst, mp_srcptr restrict numa, mp_size_t na, mp_ptr restrict tp) {
    lmmp_param_assert(na > 0);
    lmmp_param_assert(dst != NULL && tp != NULL && numa != NULL);
    lmmp_param_assert(numa[na - 1] != 0);
    lmmp_sqr_(tp, numa, na);
    lmmp_mul_(dst, tp, 2 * na, numa, na);
    na *= 3;
    while (dst[na - 1] == 0) --na;
    return na;
}

mp_limb_t lmmp_cbrtapprox_3_(mp_limb_t a0, mp_limb_t a1, mp_limb_t a2) {
    lmmp_param_assert(a1 > 0);
    mp_limb_t x[2];
    /* exact high 65 bits */
    mp_limb_t a_hi;
    mp_bitcnt_t bits;
    if (a2 == 0) {
        mp_bitcnt_t a1_bits = lmmp_limb_bits_(a1);
        bits = LIMB_BITS + a1_bits;
        a1_bits--;
        if (a1_bits == 0)
            a_hi = a0;
        else
            a_hi = (a1 << (LIMB_BITS - a1_bits)) | (a0 >> a1_bits);
    } else {
        mp_bitcnt_t a2_bits = lmmp_limb_bits_(a2);
        bits = LIMB_BITS * 2 + a2_bits;
        a2_bits--;
        if (a2_bits == 0)
            a_hi = a1;
        else
            a_hi = (a2 << (LIMB_BITS - a2_bits)) | (a1 >> a2_bits);
    }
    lmmp_debug_assert(bits >= 65);

    x[1] = bits - 1;
    x[0] = log2_fixed_64(a_hi);

    mp_limb_t r = lmmp_div_1_(x, x, 2, 3);
    if (2 * r >= 3) // round
        lmmp_inc(x);

    mp_bitcnt_t shift = x[1];
    x[0] = exp2_fixed_64(x[0]);

    lmmp_debug_assert(shift <= 64);
    if (shift == 64)
        return LIMB_MAX;
    else
        return (x[0] >> (64 - shift)) | (1ULL << shift);
}

mp_limb_t lmmp_cbrt_3_(mp_limb_t a0, mp_limb_t a1, mp_limb_t a2) {
    lmmp_param_assert(a1 > 0);

    mp_limb_t r = lmmp_cbrtapprox_3_(a0, a1, a2);
    if (r == LIMB_MAX)
        return LIMB_MAX;
    mp_limb_t t[3], a[3] = {a0, a1, a2};
    lmmp_cube_3_(t, r + 1);
    int cmp = lmmp_cmp_(t, a, 3);
    // approx的结果至多只会低估1
    if (cmp <= 0)
        return r + 1;
    else
        return r;
}

void lmmp_cbrtapprox_6_(mp_ptr dst, mp_srcptr numa, mp_size_t na) {
    lmmp_param_assert(na > 3 && na <= 6);
    lmmp_param_assert(dst != NULL && numa != NULL);
    lmmp_param_assert(numa[na - 1] != 0);
    /* extract the first 129 bits */
    int bits = lmmp_limb_bits_(numa[na - 1]);
    mp_bitcnt_t n = bits - 1;
    mp_limb_t high, low;
    if (bits == 1) {
        high = numa[na - 2];
        low = numa[na - 3];
    } else {
        bits--;
        high = (numa[na - 1] << (64 - bits)) | (numa[na - 2] >> bits);
        low = (numa[na - 2] << (64 - bits)) | (numa[na - 3] >> bits);
    }

    n += LIMB_BITS * (na - 1);
    mp_limb_t x[3] = {0, 0, n};

    log2_fixed_128(x, high, low);
    mp_limb_t r = lmmp_div_1_(x, x, 3, 3);
    if (2 * r >= 3) // round
       lmmp_inc(x);

    n = x[2];
    high = x[1];
    low = x[0];

    exp2_fixed_128(x, high, low);

    lmmp_debug_assert(n >= 64 && n <= 128);
    if (n == 64) {
        dst[0] = x[1];
        dst[1] = 1;
    } else if (n < 128) {
        n -= 64;
        mp_limb_t t = 1ULL << n;
        dst[1] = (x[1] >> (64 - n)) | t;
        dst[0] = (x[1] << n) | (x[0] >> (64 - n));
    } else {
        dst[1] = LIMB_MAX;
        dst[0] = LIMB_MAX;
    }
}

void lmmp_cbrt_6_(mp_ptr dst, mp_srcptr numa, mp_size_t na) {
    mp_limb_t ret[2];
    lmmp_cbrtapprox_6_(ret, numa, na);

    if (ret[1] == LIMB_MAX && ret[0] == LIMB_MAX) {
        dst[0] = LIMB_MAX;
        dst[1] = LIMB_MAX;
    } else {
        mp_limb_t r[2];
        r[0] = ret[0] + 1;
        r[1] = ret[1] + (r[0] == 0 ? 1 : 0);
        mp_limb_t t[6];
        mp_size_t tn = lmmp_cube_6_(t, r);
        if (tn > na) {
            dst[0] = ret[0];
            dst[1] = ret[1];
        } else {
            lmmp_debug_assert(tn == na);
            int cmp = lmmp_cmp_(t, numa, na);
            // approx的结果至多只会低估1
            if (cmp <= 0) {
                dst[0] = r[0];
                dst[1] = r[1];
            } else {
                dst[0] = ret[0];
                dst[1] = ret[1];
            }
        }
    }
}

mp_size_t lmmp_cbrtapprox_(mp_ptr restrict dst, mp_srcptr restrict numa, mp_size_t na, mp_size_t ni) {
    lmmp_param_assert(na > 0);
    lmmp_param_assert(dst != NULL && numa != NULL);
    mp_size_t n = ni + na;
    if (n == 1) {
        mp_limb_t a_cbrt = lmmp_cbrt_ulong_(numa[0]);
        dst[0] = a_cbrt;
        return 1;
    } else if (n <= 3) {
        if (ni == 0)
            dst[0] = lmmp_cbrtapprox_3_(numa[0], numa[1], numa[2]);
        else if (ni == 1)
            dst[0] = lmmp_cbrtapprox_3_(0, numa[0], numa[1]);
        else
            dst[0] = lmmp_cbrtapprox_3_(0, 0, numa[0]);
        return 1;
    } else if (n <= 6) {
        mp_limb_t a[6];
        mp_size_t i;
        for (i = 0; i < ni; i++) {
            a[i] = 0;
        }
        for (mp_size_t j = 0; i < n; i++, j++) {
            a[i] = numa[j];
        }
        lmmp_cbrtapprox_6_(dst, a, n);
        return 2;
    } else {
        /*
                A     = Ah * B^(3*lo) + Al

                Ahr   = floor(Ah^(1/3))
                x_k   = Ahr * B^lo

                x_k+1 = (2*x_k + A / x_k^2 ) / 3
        */
        mp_size_t nhi = (n - 1) / 2;
        mp_size_t nlo = nhi - (nhi % 3);
        nhi = n - nlo;
        /*
         * 设 beta = B^b，alpha = Ah^(1/n)，rho = A^(1/n)。
         * 已知 tk 满足 floor(Ah^(1/n)) - 1 <= tk <= floor(Ah^(1/n))，
         * 因此 xk = tk * beta 与真实根 rho 的误差满足（最坏情况）：
         *     e = rho - xk <= 2 * beta。
         *
         * 牛顿迭代实数形式为：
         *     F(x) = ((n-1)*x + A / x^(n-1)) / n。
         * 令 x = rho - e，在 rho 处泰勒展开可得：
         *     F(x) - rho = (n(n-1)/2) * (e^2 / rho) + O(e^3 / rho^2)。
         *
         * 由于 rho >= alpha * beta（因为 A >= Ah * beta^n，开方后略大于 alpha*beta），
         * 将 e <= 2*beta 代入，忽略高阶小量，得到误差上限：
         *     F(x) - rho <= (n(n-1)/2) * ((2*beta)^2 / (alpha*beta))
         *                 = (2*n*(n-1)*beta) / alpha。
         *
         * 为使得最终取整后的绝对误差控制在 1 以内，只需令上式 <= 1：
         *     (2*n*(n-1)*beta) / alpha <= 1
         *  =>  alpha >= 2*n*(n-1)*beta。
         *
         * 两边同时取 n 次方：
         *     Ah >= [2*n*(n-1)]^n * beta^n
         *        = [2*n*(n-1)]^n * B^(n*b)。
         *
         * 因此，若要保证递归校正始终收敛且误差不超过 1，常数 m 至少取：
         *     m_min = [2*n*(n-1)]^n。
         *
         * 对于立方根，即有：m_min = 1728
         * 
         * 注：在实践观察中，即使 m 取 1，随机输入的情况几乎不可能出现误差超过1的情况。
         *     可能存在精心构造的输入，使得迭代不收敛。
         */

#if LAMMP_DEBUG_ASSERT_CHECK == 1
        if (nhi == nlo + 1) {
            lmmp_debug_assert(numa[na - 1] > 1728);
        }
#endif
        nlo /= 3;
        lmmp_debug_assert(nlo > 0);

        TEMP_DECL;
        mp_size_t rn, Adivn = nhi + nlo;
        mp_ptr restrict Adiv;

#define Ahr (dst + nlo)
        if (ni >= 3 * nlo) {
            //  __________________ n ___________________
            //  |__________ na __________|_____ ni ____|
            //  |xxxxxxxxxxxxxxxxxxxxxxxx|0000000000000|
            //  |___________ nhi __________|__ 3*nlo __|

            rn = lmmp_cbrtapprox_(Ahr, numa, na, ni - 3 * nlo);

            Adiv = TALLOC_TYPE(Adivn, mp_limb_t);
            lmmp_zero(Adiv, ni - 2 * nlo);
            lmmp_copy(Adiv + ni - 2 * nlo, numa, na);

        } else if (ni >= 2 * nlo) {
            //  __________________ n ___________________
            //  |__________ na __________|_____ ni ____|
            //  |xxxxxxxxxxxxxxxxxxxxxxxx|0000000000000|
            //  |________ nhi _____|_nlo___|__ 2*nlo __|

            rn = lmmp_cbrtapprox_(Ahr, numa + na - nhi, nhi, 0);

            Adiv = TALLOC_TYPE(Adivn, mp_limb_t);
            lmmp_zero(Adiv, ni - 2 * nlo);
            lmmp_copy(Adiv + ni - 2 * nlo, numa, na);

        } else {
            //  __________________ n ___________________
            //  |__________ na __________|_____ ni ____|
            //  |xxxxxxxxxxxxxxxxxxxxxxxx|0000000000000|
            //  |_____ nhi ____|__nlo__|____ 2*nlo ____|

            rn = lmmp_cbrtapprox_(Ahr, numa + na - nhi, nhi, 0);

            Adiv = TALLOC_TYPE(Adivn, mp_limb_t);
            lmmp_copy(Adiv, numa + na - Adivn, Adivn);
        }

        mp_size_t Ahr2n = rn * 2;
        mp_ptr restrict Ahr2 = TALLOC_TYPE(Ahr2n, mp_limb_t);
        lmmp_sqr_(Ahr2, Ahr, rn);
        Ahr2n -= (Ahr2[Ahr2n - 1] == 0) ? 1 : 0;

        lmmp_debug_assert(Adivn >= Ahr2n);
        mp_size_t qn = Adivn - Ahr2n + 1;
        mp_size_t rkdivn = (n + 2) / 3 + 2; // 额外多两个limb，因为需要作为加法缓冲区
        mp_ptr restrict rkdiv = TALLOC_TYPE(rkdivn, mp_limb_t);

        lmmp_debug_assert(qn <= rkdivn);
        lmmp_zero(rkdiv + qn, rkdivn - qn); //高位清零

        lmmp_div_(rkdiv, NULL, Adiv, Adivn, Ahr2, Ahr2n);

        mp_limb_t cy = lmmp_shl_(Ahr, Ahr, rn, 1);
        Ahr[rn] = cy;
        rn += cy > 0 ? 1 : 0;

        lmmp_debug_assert(rn + nlo + 1 <= rkdivn);
        cy = lmmp_add_n_(rkdiv + nlo, Ahr, rkdiv + nlo, rn);
        rn += nlo;
        rkdiv[rn] = cy;
        rn += cy > 0 ? 1 : 0;

        lmmp_div_1_(dst, rkdiv, rn, 3);

        rn -= dst[rn - 1] == 0 ? 1 : 0;

        TEMP_FREE;
        return rn;
    }
#undef Ahr
}

