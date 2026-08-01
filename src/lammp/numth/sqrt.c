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

#include "../../../include/lammp/impl/mparam.h"
#include "../../../include/lammp/impl/tmp_alloc.h"
#include "../../../include/lammp/impl/inlines.h"
#include "../../../include/lammp/lmmpn.h"
#include "../../../include/lammp/numth.h"


/**
 * @brief 分治法计算整数平方根
 * @param dsts 输出：平方根的整数部分，长度为 ns。
 * @param numa 输入/输出：被开方数，长度为 2*ns。
 *               - 输入时存放原被开方数。
 *               - 返回时，若 nsh==0，低 ns 个 limb 存放余数。
 * @param ns    平方根占用的 limb 数。
 * @param nsh   右移位数（0 <= nsh < LIMB_BITS），仅顶层调用时有效。表示最终结果需要右移 nsh 位。
 * @warning ns>0, numa[2*ns-1]>=B/4, 0<=nsh<LIMB_BITS, dsts!=NULL, numa!=NULL
 * @return 返回值语义取决于 nsh 和计算路径：
 *         - 若 nsh == 0：
 *           返回余数的高位 limb（rh），余数低位部分写入 [numa,ns]。
 *         - 若 nsh > 0：
 *           1. 如果 dsts[0] 的低 nsh 位非零（即右移会丢弃有效低位），
 *              函数提前终止，直接返回 1（固定哨兵值）。
 *              此时 numa 中的余数未经计算，不可使用；返回值 1 不代表余数高位。
 *              （此优化用于调用者不需要余数的情况，避免昂贵的余数计算。）
 *           2. 如果低 nsh 位全为零，则继续计算精确余数，
 *              并返回真实的余数高位 limb（rh），同时余数低 ns 位写入 numa。
 *              但鉴于调用者通常不关心余数，该返回值可能被忽略。
 * @note 本函数被设计为递归使用，递归层级均传递 nsh=0，因此 nsh>0 的情形只可能
 *       出现在最外层调用，且通常伴随调用者不需要余数（如 lmmp_sqrt_ 中 dstr==NULL）。
 */
static mp_limb_t lmmp_sqrt_divide_(mp_ptr dsts, mp_ptr numa, mp_size_t ns, int nsh) {
    lmmp_param_assert(ns > 0);
    lmmp_param_assert(nsh >= 0 && nsh < LIMB_BITS);
    lmmp_param_assert(numa != NULL && dsts != NULL);
    lmmp_param_assert(numa[2 * ns - 1] >= LIMB_B_4);
    mp_slimb_t rh;
    if (ns == 1) {
        rh = lmmp_sqrt_2_(dsts, numa, numa);
    } else {
        mp_size_t lo = ns / 2, hi = ns - lo;
        mp_limb_t qh = lmmp_sqrt_divide_(dsts + lo, numa + 2 * lo, hi, 0);
        if (qh)
            lmmp_sub_n_(numa + 2 * lo, numa + 2 * lo, dsts + lo, hi);
        qh += lmmp_div_s_(dsts, numa + lo, ns, dsts + lo, hi);
        rh = lmmp_shr_c_(dsts, dsts, lo, 1, qh << (LIMB_BITS - 1));
        // now dsts is either correct or 1 too big,
        // if nsh-LSBs are non-zero, subtracting 1
        // will not affect anything after de-normalization
        if (dsts[0] & (((mp_limb_t)1 << nsh) - 1))
            return 1;
        if (rh)
            rh = lmmp_add_n_(numa + lo, numa + lo, dsts + lo, hi);
        qh >>= 1;
        lmmp_sqr_(numa + ns, dsts, lo);
        mp_limb_t b = qh + lmmp_sub_n_(numa, numa, numa + ns, lo * 2);
        if (lo == hi)
            rh -= b;
        else
            rh -= lmmp_sub_1_(numa + 2 * lo, numa + 2 * lo, 1, b);
        if (rh < 0) {
            qh = lmmp_add_1_(dsts + lo, dsts + lo, hi, qh);
            rh += 2 * qh + lmmp_addshl1_n_(numa, numa, dsts, ns);
            rh -= lmmp_sub_1_(numa, numa, ns, 1);
            qh -= lmmp_sub_1_(dsts, dsts, ns, 1);
        }
    }
    return rh;
}

/**
 * @brief 计算逆平方根 [dstis,ns+1]=floor(sqrt(B^(2*ns+na)/[numa,na]))-[0|1], dstis[ns]=1
 * @param dstis 目标数组
 * @param ns dsts数组的 limb 长度为 ns+1
 * @param numa 输入数组
 * @param na numa数组的 limb 长度
 * @warning ns>0, na>0, numa[na-1]>=B/4, dstis!=NULL, numa!=NULL
 * @note [dstis,ns+1]=floor(sqrt(B^(2*ns+na)/[numa,na]))-[0|1], dstis[ns]=1
 */
static void lmmp_invsqrt_newton_(mp_ptr dstis, mp_size_t ns, mp_srcptr numa, mp_size_t na) {
    lmmp_param_assert(ns >= 3);
    lmmp_param_assert(na > 0);
    lmmp_param_assert(numa != NULL && dstis != NULL);
    lmmp_param_assert(numa[na - 1] >= LIMB_B_4);
    mp_size_t nr = ns, namax = na, mn;
    mp_size_t sizes[LIMB_BITS], *sizp = sizes;

    do {
        *sizp = nr;
        nr = (nr >> 1) + 1;
        ++sizp;
    } while (nr > 2);

    numa += na;
    dstis += ns;

    // nr=2
    // i2=floor((B^5-1)/(1+floor(sqrt(x*B^4))))
    mp_limb_t numa2[6], sval[3];
    lmmp_zero(numa2, 4);
    numa2[5] = numa[-1];
    if (na > 1)
        numa2[4] = numa[-2];
    else
        numa2[4] = 0;
    lmmp_sqrt_divide_(sval, numa2, 3, 0);
    lmmp_inc(sval);
    for (mp_size_t i = 0; i < 5; ++i) numa2[i] = LIMB_MAX;
    dstis[0] = lmmp_div_s_(dstis - 2, numa2, 5, sval, 3);

    TEMP_DECL;
    mp_limb_t alloc_size = na + 2 * ns + 6;
    mp_ptr xp = TALLOC_TYPE(alloc_size, mp_limb_t);
    do {
        na = *--sizp;

        // ar = 0:[numa-nr,nr]
        // an = 0:[numa-na,na]
        // ir = 1:[dst-nr,nr] = floor(B^(3*nr/2)/sqrt(ar)) - [0|1]
        //  d = B^(na+2*nr)-an*ir*ir
        //  -4*B^(na+nr) < d < 4*B^(na+nr)

        mp_size_t naz = LMMP_MIN(na, namax);
        // mp_size_t zeros = na - naz;
        mp_size_t nsqr, nres = naz + nr + 1;
        mp_ptr dp = xp + 2 * nr + 1, dip = xp + nr + 1;
        int cmod;  // 1=mod b^mn-1, 0=mod b^(naz+nr+1)
        int sign;  // 1:d<0, 0:d>=0
        mn = lmmp_fft_next_size_(nres);

        // ir^2
        if (2 * SQRT_NEWTON_MODM_THRESHOLD + mn >= nr * 2 + 1) {
            cmod = 0;
            lmmp_sqr_(xp, dstis - nr, nr + 1);
            nsqr = 2 * nr + 1;
        } else {
            cmod = 1;
            lmmp_mul_mersenne_(xp, mn, dstis - nr, nr + 1, dstis - nr, nr + 1);
            nsqr = mn;
        }

        // ir^2*an
        if (naz < SQRT_NEWTON_MODM_THRESHOLD || naz * 8 < nsqr || mn >= nsqr + naz) {
            if (cmod == 0)
                nsqr = LMMP_MIN(nsqr, nres);
            lmmp_mul_(dp, xp, nsqr, numa - naz, naz);
            if (cmod == 1) {
                if (lmmp_add_(dp, dp, mn, dp + mn, naz))
                    lmmp_inc(dp);
            }
        } else {
            if (nsqr > mn) {  // cmod==0
                if (lmmp_add_(xp, xp, mn, xp + mn, nsqr - mn))
                    lmmp_inc(xp);
            }
            lmmp_mul_mersenne_(dp, mn, xp, nsqr, numa - naz, naz);
            cmod = 1;
        }

        if (cmod == 1) {
            // naz+nr < mn <= naz+2*nr
            //[dp,mn] -= B^(naz+2*nr) mod (B^mn-1)
            dp[mn] = 1;
            lmmp_dec(dp + naz + 2 * nr - mn);
            if (dp[mn] == 0)
                lmmp_dec(dp);
        }

        if (dp[nres - 1] > 3) {  //-d<0
            if (cmod == 0)
                lmmp_dec(dp);  // for neg to not
            // else (neg to not) compensate (mod transfer)
            dp += naz;
            lmmp_shlnot_(xp, dp + 1, nr, LIMB_BITS - 1);
            xp[0] ^= dp[0] >> 1;
            xp[nr] = ~dp[nr] >> 1;
            sign = 0;
        } else {  //-d>0
            lmmp_shr_(xp, dp + naz, nr + 1, 1);
            if ((dp[naz] & 1) || !lmmp_zero_q_(dp, naz))
                lmmp_inc(xp);
            sign = 1;
        }

        lmmp_mul_n_(dip, xp, dstis - nr, nr + 1);

        if (sign) {
            if (lmmp_zero_q_(dip, 3 * nr - na)) {
                // a limit for dec
                dip[2 * nr + 1] = 1;
                lmmp_dec(dip + 3 * nr - na);
            }
            lmmp_not_(dstis - na, dip + 3 * nr - na, na - nr);
            lmmp_dec_1(dstis - nr, dip[2 * nr] + 1);
        } else {
            lmmp_copy(dstis - na, dip + 3 * nr - na, na - nr);
            lmmp_inc_1(dstis - nr, dip[2 * nr]);
        }

        nr = na;
    } while (sizp != sizes);
    TEMP_FREE;
}

/**
 * @brief 计算近似平方根 [dsts,nf+na/2+1]=[floor|round](sqrt([numa,na]*B^(2*nf)))
 * @param dsts 目标数组
 * @param numa 输入数组
 * @param na numa数组的 limb 长度
 * @param nf 精度因子
 * @warning na>0, nf>=2, dsts!=NULL, numa!=NULL, eqsep(dsts,numa)
 */
static void lmmp_sqrt_newton_(mp_ptr dsts, mp_srcptr numa, mp_size_t na, mp_size_t nf) {
    lmmp_param_assert(na > 0);
    lmmp_param_assert(nf >= 2);
    lmmp_param_assert(numa != NULL && dsts != NULL);
    mp_limb_t high = numa[na - 1];
    int nsh = lmmp_leading_zeros_(high) / 2;
    mp_size_t ns = na / 2 + 1 + nf;

    TEMP_DECL;
    mp_limb_t alloc_size = (nsh ? na : 0) + ns + 1;
    mp_ptr tp = TALLOC_TYPE(alloc_size, mp_limb_t), numa2;
    if (nsh) {
        numa2 = tp;
        lmmp_shl_(numa2, numa, na, nsh * 2);
        tp += na;
    } else
        numa2 = (mp_ptr)numa;

    lmmp_invsqrt_newton_(tp, ns, numa2, na);

    mp_ptr msqr = TALLOC_TYPE(na + ns + 1, mp_limb_t);

    if (ns + 1 > na)
        lmmp_mul_(msqr, tp, ns + 1, numa2, na);
    else
        lmmp_mul_(msqr, numa2, na, tp, ns + 1);

    mp_limb_t cceil;
    if (na & 1) {
        nsh += LIMB_BITS / 2;
        lmmp_shr_(dsts, msqr + na, ns, nsh);
        cceil = msqr[na] >> (nsh - 1);
    } else {
        if (nsh) {
            lmmp_shr_(dsts, msqr + na + 1, ns - 1, nsh);
            cceil = msqr[na + 1] >> (nsh - 1);
        } else {
            lmmp_copy(dsts, msqr + na + 1, ns - 1);
            cceil = msqr[na] >> (LIMB_BITS - 1);
        }
        dsts[ns - 1] = 0;
    }

    if (cceil & 1)
        lmmp_inc(dsts);

    TEMP_FREE;
}

void lmmp_sqrt_(mp_ptr dsts, mp_ptr dstr, mp_srcptr numa, mp_size_t na, mp_size_t nf) {
    lmmp_debug_assert(na > 0);
    lmmp_debug_assert(numa[na - 1] > 0);
    mp_limb_t high = numa[na - 1];
    int nsh = lmmp_leading_zeros_(high) / 2;
    mp_size_t nl = na + 2 * nf;
    if (nl == 1) {
        mp_limb_t srt;
        lmmp_sqrt_1_(&srt, high << nsh * 2);
        srt >>= nsh;
        dsts[0] = srt;
        if (dstr)
            dstr[0] = high - srt * srt;
    } else if (!dstr && nf >= 10 * na + SQRT_NEWTON_THRESHOLD) {
        lmmp_sqrt_newton_(dsts, numa, na, nf);
    } else {
        TEMP_DECL;
        mp_limb_t ns = (nl + 1) / 2;
        mp_ptr numa2 = TALLOC_TYPE(2 * ns, mp_limb_t);
        if (nf)
            lmmp_zero(numa2, 2 * nf);
        if (nsh)
            lmmp_shl_(numa2 + 2 * ns - na, numa, na, nsh * 2);
        else
            lmmp_copy(numa2 + 2 * ns - na, numa, na);
        if (nl & 1) {
            numa2[2 * nf] = 0;
            nsh += LIMB_BITS / 2;
        } else {
            dsts[ns] = 0;
        }
        mp_limb_t rh = lmmp_sqrt_divide_(dsts, numa2, ns, dstr ? 0 : nsh);
        if (nsh) {
            if (dstr) {
                mp_limb_t ds = dsts[0] & (((mp_limb_t)1 << nsh) - 1);
                rh += lmmp_addmul_1_(numa2, dsts, ns, 2 * ds);
                mp_limb_t b = lmmp_submul_1_(numa2, &ds, 1, ds);
                if (ns == 1)
                    rh -= b;
                else
                    rh -= lmmp_sub_1_(numa2 + 1, numa2 + 1, ns - 1, b);
            }
            lmmp_shr_(dsts, dsts, ns, nsh);
        }
        if (dstr) {
            numa2[ns] = rh;
            nsh *= 2;
            if (nsh >= LIMB_BITS) {
                nsh -= LIMB_BITS;
                ++numa2;
            } else
                ++ns;
            if (nsh)
                lmmp_shr_(dstr, numa2, ns, nsh);
            else
                lmmp_copy(dstr, numa2, ns);
        }
        TEMP_FREE;
    }
}
