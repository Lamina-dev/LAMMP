/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../../../include/lammp/impl/ele_mul.h"
#include "../../../include/lammp/impl/mparam.h"
#include "../../../include/lammp/impl/tmp_alloc.h"
#include "../../../include/lammp/impl/longlong.h"
#include "../../../include/lammp/lmmpn.h"
#include "../../../include/lammp/numth.h"

mp_size_t lmmp_arith_seqprod_size_(uint x, uint n, uint m) {
    double x_m = (double)x / m;
    double log_l = (n + 1) * log(m) + lgamma(x_m + n + 1) - lgamma(x_m);
    double log2_l = log_l / LOG2_;
    mp_size_t rn = ceil(log2_l / LIMB_BITS) + 2; /* more two limbs */
    return rn;
}

static inline mp_size_t _odd_pow_(mp_ptr dst, mp_size_t rn, uint base, ulong exp) {
    if (base <= 0xf)
        return lmmp_u4_pow_1_(dst, rn, base, exp);
    else if (base <= MP_UCHAR_MAX)
        return lmmp_u8_pow_1_(dst, rn, base, exp);
    else if (base <= MP_USHORT_MAX)
        return lmmp_u16_pow_1_(dst, rn, base, exp);
    else
        return lmmp_u32_pow_1_(dst, rn, base, exp);
}

static inline mp_size_t _odd_nPr_(mp_ptr restrict dst, mp_size_t rn, ulong n, ulong r) {
    if (n <= NPR_SHORT_LIMIT) {
        return lmmp_odd_nPr_ushort_(dst, rn, n, r);
    } else {
        return lmmp_odd_nPr_uint_(dst, rn, n, r);
    }
}

/*
当x=p*m时，
x(x+m)...(x+n*m) = p*m * (p+1)*m * ... * (p+n)*m
                 = m^(n+1) * p(p+1)...(p+n)
分别计算幂和组合数，然后相乘
*/
static inline mp_size_t pow_nPr_(mp_ptr restrict dst, mp_size_t rn, uint x, uint n, uint m) {
    TEMP_DECL;
    uint p = x / m;

    mp_bitcnt_t bits;
    mp_size_t tn = lmmp_nPr_size_(p + n, n + 1, &bits);
    tn -= bits / LIMB_BITS;
    mp_ptr restrict tp = TALLOC_TYPE(tn, mp_limb_t);
    tn = _odd_nPr_(tp, tn, p + n, n + 1);

    mp_size_t tz = lmmp_tailing_zeros_(m);
    m >>= tz;
    mp_size_t mpown = lmmp_pow_1_size_(m, n + 1);
    mp_ptr restrict mpow = TALLOC_TYPE(mpown, mp_limb_t);
    mpown = _odd_pow_(mpow, mpown, m, n + 1);

    bits += tz * (n + 1);
    mp_size_t shw = bits / LIMB_BITS;
    bits %= LIMB_BITS;

    lmmp_zero(dst, shw);
    if (tn >= mpown)
        lmmp_mul_(dst + shw, tp, tn, mpow, mpown);
    else
        lmmp_mul_(dst + shw, mpow, mpown, tp, tn);

    rn = tn + mpown;
    rn -= dst[shw + rn - 1] == 0 ? 1 : 0;
    if (bits > 0) {
        dst[shw + rn] = lmmp_shl_(dst + shw, dst + shw, rn, bits);
        rn += shw + 1;
        rn -= dst[rn - 1] == 0 ? 1 : 0;
    } else {
        rn += shw;
    }
    TEMP_FREE;
    return rn;
}

mp_size_t lmmp_arith_seqprod_(mp_ptr restrict dst, mp_size_t rn, uint x, uint n, uint m) {
    lmmp_param_assert(dst != NULL);
    lmmp_param_assert(rn >= 1);
    lmmp_param_assert(x >= 1);
    lmmp_param_assert(n >= 1);
    lmmp_param_assert(m > 1);

    if (x % m == 0) {
        return pow_nPr_(dst, rn, x, n, m);
    }

    mp_bitcnt_t bits = 0;
    while ((x & 1) == 0 && (m & 1) == 0) {
        x >>= 1;
        m >>= 1;
        bits++;
    }
    bits *= n + 1;

    TEMP_DECL;
    ulongp restrict limbs = TALLOC_TYPE((n + 1) / 2 + 1, ulong);
    mp_size_t limbn = 0;
    ulong t = 1, s, v;
    mp_bitcnt_t cnt;
    for (uint i = 0; i <= n; i++) {
        s = x + i * m;
        ctz_shr_u64(v, s, cnt);
        t *= v;
        bits += cnt;
        if (t > MP_UINT_MAX) {
            limbs[limbn++] = t;
            t = 1;
        }
    }
    ctz_shr_u64(v, t, cnt);
    bits += cnt;
    if (v != 1) {
        limbs[limbn++] = v;
    }
    mp_size_t shw = bits / LIMB_BITS;
    bits %= LIMB_BITS;
    mp_ptr restrict b = TALLOC_TYPE(limbn * 2, mp_limb_t);
    mp_size_t bn = lmmp_elem_mul_ulong_(b, limbs, limbn, b + limbn);
    lmmp_zero(dst, shw);
    if (bits > 0) {
        dst[shw + bn] = lmmp_shl_(dst + shw, b, bn, bits);
        rn = bn + shw + 1;
        rn -= dst[rn - 1] == 0 ? 1 : 0;
    } else {
        lmmp_copy(dst + shw, b, bn);
        rn = bn + shw;
    }
    TEMP_FREE;
    return rn;
}