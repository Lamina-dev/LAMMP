/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../../../include/lammp/numth.h"
#include "../../../include/lammp/lmmpn.h"
#include "../../../include/lammp/impl/longlong.h"

/* from https://arxiv.org/abs/2204.04342 */

static const unsigned char binv_tab[128] = {
    1,   171, 205, 183, 57,  163, 197, 239, 241, 27,  61,  167, 41,  19,  53,  223, 225, 139, 173, 151, 25,  131,
    165, 207, 209, 251, 29,  135, 9,   243, 21,  191, 193, 107, 141, 119, 249, 99,  133, 175, 177, 219, 253, 103,
    233, 211, 245, 159, 161, 75,  109, 87,  217, 67,  101, 143, 145, 187, 221, 71,  201, 179, 213, 127, 129, 43,
    77,  55,  185, 35,  69,  111, 113, 155, 189, 39,  169, 147, 181, 95,  97,  11,  45,  23,  153, 3,   37,  79,
    81,  123, 157, 7,   137, 115, 149, 63,  65,  235, 13,  247, 121, 227, 5,   47,  49,  91,  125, 231, 105, 83,
    117, 31,  33,  203, 237, 215, 89,  195, 229, 15,  17,  59,  93,  199, 73,  51,  85,  255};

uint lmmp_binvert_uint_(uint a) {
    lmmp_param_assert(a % 2 == 1);
    ulong r, y;

    r = binv_tab[(a / 2) & 0x7F]; /* 8 bits */
    y = 1 - a * r;
    r = r * (1 + y); /* 16 bits */
    y *= y;
    r = r * (1 + y); /* 32 bits */
    return r;
}

ulong lmmp_binvert_ulong_(ulong a) {
    lmmp_param_assert(a % 2 == 1);
    ulong r, y;

    r = binv_tab[(a / 2) & 0x7F]; /* 8 bits */
    y = 1 - a * r;
    r = r * (1 + y); /* 16 bits */
    y *= y;
    r = r * (1 + y); /* 32 bits */
    y *= y;
    r = r * (1 + y); /* 64 bits */
    return r;
}

void lmmp_binvert_2_(mp_ptr dst, mp_srcptr numa) {
    lmmp_param_assert(numa[0] % 2 == 1);
    mp_limb_t k, t;
    mp_limb_t a1 = numa[1];
    mp_limb_t a0 = numa[0];
    mp_limb_t xn = lmmp_binvert_ulong_(a0);
    mp_limb_t z;
    /*
     xn * a0 == 1 + k * B
     yn := xn * (2 - a * xn) mod B^2
        := xn * (2 - a0 * xn - a1 * xn * B) mod B^2
        := xn * (2 - 1 - k*B - a1 * xn * B) mod B^2
        := xn * (1 - k*B - a1 * xn * B) mod B^2
        := (xn - xn*k * B - a1 * xn^2 * B) mod B^2
    */
    _umul64to128_(a0, xn, &t, &k);
    z = xn * k;
    z += a1 * xn * xn;
    dst[0] = xn;
    dst[1] = -z;
}

static inline void _umul128to192_(uint64_t a_high, uint64_t a_low, uint64_t b_high, uint64_t b_low, uint64_t rr[3]) {
    uint64_t p1_low, p1_high;  // p1 = a_low × b_high
    uint64_t p2_low, p2_high;  // p2 = a_high × b_low
    _umulx64to128_(a_low, b_low, rr, rr + 1);
    _umulx64to128_(a_low, b_high, &p1_low, &p1_high);
    _umulx64to128_(a_high, b_low, &p2_low, &p2_high);
    /*
        | res0 | res1 | res2 |
        |  p0l |  p0h |      |
               |  p1l |  p1h |
               |  p2l |  p2h |
               |      |  p3l |
    */
    rr[1] += p1_low;
    uint64_t carry = (rr[1] < p1_low) ? 1 : 0;
    rr[1] += p2_low;
    carry += (rr[1] < p2_low) ? 1 : 0;

    rr[2] = a_high * b_high;
    rr[2] += carry;
    rr[2] += p1_high;
    rr[2] += p2_high;
}

void lmmp_binvert_3_(mp_ptr restrict dst, mp_srcptr restrict numa) {
    /*
           a == a0 + a1 * B^2
     xn * a0 == 1 + k * B^2
     yn := xn * (2 - a * xn) mod B^4
        := xn * (2 - a0 * xn - a1 * xn * B^2) mod B^4
        := xn * (2 - 1 - k*B^2 - a1 * xn * B^2) mod B^4
        := xn * (1 - k*B^2 - a1 * xn * B^2) mod B^4
        := (xn - xn*k * B^2 - a1 * xn^2 * B^2) mod B^4
    */
    lmmp_binvert_2_(dst, numa);
    mp_limb_t k[3];
    mp_limb_t z;
    mp_limb_t a2 = numa[2];
    _umul128to192_(dst[1], dst[0], numa[1], numa[0], k);
    lmmp_debug_assert(k[1] == 0 && k[0] == 1);
#define xn (dst[0])
#define k (k[2])
    z = xn * k;
    z += a2 * xn * xn;
    dst[2] = -z;
#undef xn
#undef k
}

void lmmp_binvert_4_(mp_ptr restrict dst, mp_srcptr restrict numa) {
    /*
           a == a0 + a1 * B^2
     xn * a0 == 1 + k * B^2
     yn := xn * (2 - a * xn) mod B^4
        := xn * (2 - a0 * xn - a1 * xn * B^2) mod B^4
        := xn * (2 - 1 - k*B^2 - a1 * xn * B^2) mod B^4
        := xn * (1 - k*B^2 - a1 * xn * B^2) mod B^4
        := (xn - xn*k * B^2 - a1 * xn^2 * B^2) mod B^4
    */
    lmmp_binvert_2_(dst, numa);
    mp_limb_t k[4];
    mp_limb_t z[2];
    mp_limb_t t[2];
    _umul128to256_(dst[1], dst[0], numa[1], numa[0], k);
    lmmp_debug_assert(k[1] == 0 && k[0] == 1);

#define xn (dst)
#define k (k + 2)
    _umul128to128_(k[1], k[0], xn[1], xn[0], z);
    
    _umul64to128_(xn[0], xn[0], t, t + 1);
    t[1] += (xn[1] * xn[0]) << 1;
    _umul128to128_(t[1], t[0], numa[3], numa[2], t);

    _u128add(z, z, t);
    dst[2] = 0;
    dst[3] = 0;
    _u128sub(dst + 2, dst + 2, z);

#undef xn
#undef k
}