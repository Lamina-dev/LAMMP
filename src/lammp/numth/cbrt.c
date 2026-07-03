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

/**
 * @brief 计算 [numa, na] 的立方
 * @param dst 目标数组（3*na个limb）
 * @param numa 源数组
 * @param na 源数组的长度
 * @param tp 临时数组（2*na个limb）
 * @return 返回结果的数组长度
 */
static inline mp_size_t lmmp_cube_(mp_ptr restrict dst, mp_srcptr restrict numa, mp_size_t na, mp_ptr restrict tp) {
    lmmp_sqr_(tp, numa, na);
    lmmp_mul_(dst, tp, 2 * na, numa, na);
    na *= 3;
    while (na > 1 && dst[na - 1] == 0) --na;
    return na;
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
        mp_limb_t t[10];
        mp_size_t tn = lmmp_cube_(t, r, 2, t + 6);
        if (tn < na) {
            dst[0] = r[0];
            dst[1] = r[1];
        } else if (tn > na) {
            dst[0] = ret[0];
            dst[1] = ret[1];
        } else if (tn == na) {
            int cmp = lmmp_cmp_(t, numa, tn);
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