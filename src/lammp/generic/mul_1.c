/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../../../include/lammp/impl/_umul64to128_.h"
#include "../../../include/lammp/lmmpn.h"

mp_limb_t lmmp_mul_1_(mp_ptr restrict dst, mp_srcptr restrict numa, mp_size_t na, mp_limb_t x) {
    mp_limb_t ul, cl, hpl, lpl;
    cl = 0;
    if (dst == numa) {
        do {
            ul = *dst;
            _umul64to128_(ul, x, &lpl, &hpl);
            lpl += cl;
            cl = (lpl < cl) + hpl;
            *dst++ = lpl;
        } while (--na != 0);
    } else {
        do {
            ul = *numa++;
            _umul64to128_(ul, x, &lpl, &hpl);
            lpl += cl;
            cl = (lpl < cl) + hpl;
            *dst++ = lpl;
        } while (--na != 0);
    }
    return cl;
}

mp_limb_t lmmp_addmul_1_(mp_ptr restrict numa, mp_srcptr restrict numb, mp_size_t n, mp_limb_t b) {
    mp_limb_t ul, cl, hpl, lpl, rl;
    cl = 0;
    if (numa == numb) {
        do {
            ul = *numa;
            _umul64to128_(ul, b, &lpl, &hpl);
            lpl += cl;
            cl = (lpl < cl) + hpl;
            lpl = ul + lpl;
            cl += lpl < ul;
            *numa++ = lpl;
        } while (--n != 0);
    } else {
        do {
            ul = *numb++;
            _umul64to128_(ul, b, &lpl, &hpl);
            lpl += cl;
            cl = (lpl < cl) + hpl;
            rl = *numa;
            lpl = rl + lpl;
            cl += lpl < rl;
            *numa++ = lpl;
        } while (--n != 0);
    }
    return cl;
}

mp_limb_t lmmp_submul_1_(mp_ptr restrict numa, mp_srcptr restrict numb, mp_size_t n, mp_limb_t b) {
    mp_limb_t ul, cl, hpl, lpl, rl;
    cl = 0;
    if (numa == numb) {
        do {
            ul = *numa;
            _umul64to128_(ul, b, &lpl, &hpl);
            lpl += cl;
            cl = (lpl < cl) + hpl;
            lpl = ul - lpl;
            cl += lpl > ul;
            *numa++ = lpl;
        } while (--n != 0);
    } else {
        do {
            ul = *numb++;
            _umul64to128_(ul, b, &lpl, &hpl);
            lpl += cl;
            cl = (lpl < cl) + hpl;
            rl = *numa;
            lpl = rl - lpl;
            cl += lpl > rl;
            *numa++ = lpl;
        } while (--n != 0);
    }
    return cl;
}
