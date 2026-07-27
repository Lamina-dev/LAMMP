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
#include "../../../include/lammp/impl/longlong.h"
#include "../../../include/lammp/numth.h"


ulong lmmp_sqrt_ulong_(ulong a) {
    ulong is;

    is = (ulong)sqrt((double)a);

    is -= (is * is > a);
    if (is == (1ULL << 32))
        is--;
    return is;
}

static mp_limb_t lmmp_sqrt_2normalize_(mp_limb_t a0, mp_limb_t a1) {
    lmmp_param_assert(a1 >= LIMB_B_4);
    mp_limb_t rl, s, q, u;
    mp_slimb_t rh;
    s = lmmp_sqrt_ulong_(a1);
    rl = a1 - s * s;

    /*
        s^2 + rl = a1
        (s*2^32 + q)^2 <= a1*2^64 + a0
        if ignore q^2, then
           q = (rl*2^31 + (a0>>33)) / s
    */

    rl = rl << 31 | a0 >> 33;
    q = rl / s;
    q -= q >> 32;

    u = rl - s * q;
    s = s << 32 | q;
    rh = u >> 31;
    rl = (u << 33) | (a0 & (((mp_limb_t)1 << 33) - 1));

    q *= q;
    rh -= rl < q;
    if (rh < 0) {
        --s;
    }

    return s;
}

mp_limb_t lmmp_sqrt_2_(mp_limb_t a0, mp_limb_t a1) {
    lmmp_param_assert(a1 > 0);
    int shift = 0;
    if (a1 < LIMB_B_4) {
        shift = lmmp_leading_zeros_(a1);
        shift -= (shift % 2);
        a1 = (a1 << shift) | (a0 >> (64 - shift));
        a0 <<= shift;
        shift /= 2;
    }
    return lmmp_sqrt_2normalize_(a0, a1) >> shift;
}

static void lmmp_sqrt_4normalize_(mp_ptr dst, mp_ptr numa) {
    lmmp_param_assert(numa[3] >= LIMB_B_4);
    lmmp_param_assert(dst != NULL && numa != NULL);

    mp_size_t sh = lmmp_sqrt_2normalize_(numa[2], numa[3]);

    mp_limb_t t[3];
    lmmp_div_1_(t, numa + 1, 3, sh);
    t[1] += sh;
    t[2] += t[1] < sh;
    t[0] = (t[0] >> 1) | (t[1] << 63);
    t[1] = (t[1] >> 1) | (t[2] << 63);
    t[2] >>= 1;
    if (t[2]) {
        dst[0] = LIMB_MAX;
        dst[1] = LIMB_MAX;
        return;
    } else {
        dst[0] = t[0];
        dst[1] = t[1];
        return;
    }
}

/**
 * @brief 计算近似平方根 floor(sqrt([numa,na]))
 * @param dst 结果指针（长度为 2 个limb）
 * @param numa 被开方数指针
 * @param na 被开方数的 limb 长度
 * @warning dst!=NULL, numa!=NULL, 2<na<=4, numa[na-1]!=0, eqsep(dst,numa)
 * @return floor(sqrt(a0+a1*B))
 */
static void lmmp_sqrtapprox_4_(mp_ptr dst, mp_srcptr numa, mp_size_t na) {
    lmmp_param_assert(na > 0);
    lmmp_param_assert(numa[na - 1] > 0);
    lmmp_param_assert(dst != NULL && numa != NULL);
    int shift;
    mp_limb_t a[4];
    if (na == 4) {
        if (numa[3] < LIMB_B_4) {
            shift = lmmp_leading_zeros_(numa[3]);
            shift -= (shift % 2);
            a[3] = (numa[3] << shift) | (numa[2] >> (64 - shift));
            a[2] = (numa[2] << shift) | (numa[1] >> (64 - shift));
            a[1] = (numa[1] << shift) | (numa[0] >> (64 - shift));
            a[0] <<= shift;
            shift /= 2;
        } else {
            a[3] = numa[3];
            a[2] = numa[2];
            a[1] = numa[1];
            a[0] = numa[0];
            shift = 0;
            lmmp_sqrt_4normalize_(dst, a);
            return;
        }
    } else {
        if (numa[2] < LIMB_B_4) {
            shift = lmmp_leading_zeros_(numa[2]);
            shift -= (shift % 2);
            a[3] = (numa[2] << shift) | (numa[1] >> (64 - shift));
            a[2] = (numa[1] << shift) | (numa[0] >> (64 - shift));
            a[1] = (numa[0] << shift);
            a[0] = 0;
            shift += 64;
            shift /= 2;
        } else {
            a[3] = numa[2];
            a[2] = numa[1];
            a[1] = numa[0];
            a[0] = 0;
            shift = 32;
        }
    }
    lmmp_sqrt_4normalize_(dst, a);
    lmmp_debug_assert(shift > 0 && shift < 64);
    dst[0] = (dst[0] >> shift) | (dst[1] << (64 - shift));
    dst[1] >>= shift;
    return;
}

void lmmp_sqrt_4_(mp_ptr dst, mp_srcptr numa, mp_size_t na) {
    lmmp_param_assert(na > 0);
    lmmp_param_assert(numa[na - 1] > 0);
    lmmp_param_assert(dst != NULL && numa != NULL);

    mp_limb_t sqr[4];
    lmmp_sqrtapprox_4_(dst, numa, na);

    _usqr128to256_(dst[0], dst[1], sqr);
    if (lmmp_cmp_(sqr, numa, na) > 0) {
        lmmp_dec(dst);
    }
}