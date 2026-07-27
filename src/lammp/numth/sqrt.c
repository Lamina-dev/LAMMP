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

#include <math.h>

#include "../../../include/lammp/impl/inlines.h"
#include "../../../include/lammp/impl/tmp_alloc.h"
#include "../../../include/lammp/lmmpn.h"
#include "../../../include/lammp/numth.h"


mp_size_t lmmp_sqrtapprox_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_size_t ni) {
    lmmp_param_assert(na > 0);
    lmmp_param_assert(dst != NULL && numa != NULL);
    lmmp_param_assert(numa[na - 1] > 0);
    mp_size_t n = na + ni;
    if (n == 1) {
        dst[0] = (mp_limb_t)sqrt((double)numa[0]);
        return 1;
    } else if (n == 2) {
        if (ni == 0)
            dst[0] = lmmp_sqrt_2_(numa[0], numa[1]);
        else
            dst[0] = lmmp_sqrt_2_(0, numa[0]);
        return 1;
    } else if (n <= 4) {
        mp_limb_t a[4];
        mp_size_t i;
        for (i = 0; i < ni; i++)
            a[i] = 0;
        for (mp_size_t j = 0; j < na; i++, j++)
            a[i] = numa[j];
        lmmp_sqrt_4_(dst, a, n);
        return 2;
    } else {
        /*
                A     = Ah * B^(2*lo) + Al

                Ahr   = floor(Ah^(1/2))
                x_k   = Ahr * B^lo

                x_k+1 = (x_k + A / x_k ) / 2
        */
        mp_size_t nhi = (n - 1) / 2;
        mp_size_t nlo = nhi - (nhi % 2);
        nhi = n - nlo;
        /**
         * 假设计算 n 次根式：A^(1/n)
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
         * 对于平方根，即有：m_min = 16
         *
         * 注：16 是一个保守的上界，不满足此上界并不意味着迭代一定不收敛。
         */
#define MMIN 16
#if LAMMP_DEBUG_ASSERT_CHECK == 1
        if (nhi == nlo + 1) {
            lmmp_debug_assert(numa[na - 1] > MMIN);
        }
#endif
        nlo /= 2;
        lmmp_debug_assert(nlo > 0);

        TEMP_DECL;
        mp_size_t rn, Adivn = nhi + nlo;
        mp_ptr restrict Adiv;

#define Ahr (dst + nlo)
        if (ni >= 2 * nlo) {
            //  __________________ n ___________________
            //  |__________ na __________|_____ ni ____|
            //  |xxxxxxxxxxxxxxxxxxxxxxxx|0000000000000|
            //  |___________ nhi __________|__ 2*nlo __|

            rn = lmmp_sqrtapprox_(Ahr, numa, na, ni - 2 * nlo);

            Adiv = TALLOC_TYPE(Adivn, mp_limb_t);
            lmmp_zero(Adiv, ni - nlo);
            lmmp_copy(Adiv + ni - nlo, numa, na);

        } else if (ni >= nlo) {
            //  __________________ n ___________________
            //  |__________ na __________|_____ ni ____|
            //  |xxxxxxxxxxxxxxxxxxxxxxxx|0000000000000|
            //  |_______ nhi ______|__ nlo __|__ nlo __|

            rn = lmmp_sqrtapprox_(Ahr, numa + na - nhi, nhi, 0);

            Adiv = TALLOC_TYPE(Adivn, mp_limb_t);
            lmmp_zero(Adiv, ni - nlo);
            lmmp_copy(Adiv + ni - nlo, numa, na);

        } else {
            //  __________________ n ___________________
            //  |_____________ na ____________|__ ni __|
            //  |xxxxxxxxxxxxxxxxxxxxxxxxxxxxx|00000000|
            //  |______ nhi _____|__ nlo __|___ nlo ___|

            rn = lmmp_sqrtapprox_(Ahr, numa + na - nhi, nhi, 0);

            Adiv = TALLOC_TYPE(Adivn, mp_limb_t);
            lmmp_copy(Adiv, numa + na - Adivn, Adivn);
        }

        mp_size_t qn = Adivn - rn + 1;
        mp_size_t rkdivn = (n + 1) / 2 + 1;  // 额外多一个limb
        mp_ptr restrict rkdiv = TALLOC_TYPE(rkdivn, mp_limb_t);

        lmmp_debug_assert(qn <= rkdivn);

        lmmp_div_(rkdiv, NULL, Adiv, Adivn, Ahr, rn);

        lmmp_debug_assert(rn + nlo <= rkdivn);
        mp_limb_t cy = lmmp_add_n_(rkdiv + nlo, rkdiv + nlo, Ahr, rn) + rkdiv[rkdivn - 1];

        rn += nlo;
        if (cy <= 1) {
            cy <<= 63;
            lmmp_shr_c_(dst, rkdiv, rn, 1, cy);
        } else {
            lmmp_debug_assert(cy == 2);
            lmmp_fill(dst, 0, rn, LIMB_MAX);
        }

        TEMP_FREE;
        return rn;
    }
#undef Ahr
}
