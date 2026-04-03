/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../../../include/lammp/impl/mpdef.h"
#include "../../../include/lammp/lmmpn.h"
#include "../../../include/lammp/matrix.h"
#include "../../../include/lammp/numth.h"

mp_size_t lmmp_arith_seqprod_(mp_ptr restrict dst, mp_size_t rn, uint x, uint n, uint m) {
    lmmp_param_assert(dst != NULL);
    lmmp_param_assert(rn >= 1);
    lmmp_param_assert(x >= 1);
    lmmp_param_assert(n >= 1);
    lmmp_param_assert(m >= 1);

    if (x % m == 0 && m != 1) {
        /* 
           x = p * m
           x(x+1)...(x+n-1) = p*m * (p+1)*m * ... * (p+n)*m
                            = m^(n+1) * p(p+1)...(p+n)
        */
        uint p = x / m;
        TEMP_B_DECL;
        mp_size_t mpown = lmmp_pow_1_size_(m, n + 1);
        mp_ptr restrict mpow = BALLOC_TYPE(mpown, mp_limb_t);
        mpown = lmmp_pow_1_(mpow, mpown, m, n + 1);
        mp_size_t tn = lmmp_nPr_size_(p + n, n + 1);
        mp_ptr restrict tp = BALLOC_TYPE(tn, mp_limb_t);
        tn = lmmp_nPr_(tp, tn, p + n, n + 1);

        mp_size_t tz = 0;
        while (*tp == 0) {
            tp++;
            tn--;
            tz++;
        }
        while (*mpow == 0) {
            mpow++;
            mpown--;
            tz++;
        }
        lmmp_zero(dst, tz);

        if (tn >= mpown)
            lmmp_mul_(dst + tz, tp, tn, mpow, mpown);
        else
            lmmp_mul_(dst + tz, mpow, mpown, tp, tn);
        TEMP_B_FREE;
        rn = tz + tn + mpown;
        rn -= dst[rn - 1] == 0 ? 1 : 0;
        return rn;
    }

    mp_size_t shl = 0;
    while ((x & 1) == 0 && (m & 1) == 0) {
        x >>= 1;
        m >>= 1;
        shl++;
    }
    shl *= n + 1;
    mp_size_t shw = shl / LIMB_BITS;
    shl %= LIMB_BITS;
    lmmp_zero(dst, shw);
    if (m == 1) {
        rn = lmmp_nPr_(dst + shw, rn - shw, n + x, n + 1);
        dst[shw + rn] = lmmp_shl_(dst + shw, dst + shw, rn, shl);
        rn += shw + 1;
        rn -= dst[rn - 1] == 0 ? 1 : 0;
        return rn;
    } else {
        mp_ptr b;
        mp_size_t bn;
        if (x + n * m <= MP_USHORT_MAX) {
            TEMP_B_DECL;
            mp_size_t tn = (n + 4) / 4;
            mp_limb_t* restrict t = BALLOC_TYPE(tn, mp_limb_t);
            mp_size_t i, j;
            mp_limb_t count = 1, prod = 1;
            for (i = 0, j = 0; j <= n; j += 4) {
                prod *= x + j * m;
                count++;
                if (count == 4) {
                    t[i++] = prod;
                    count = 1;
                    prod = 1;
                }
            }
            lmmp_debug_assert(i <= tn);
            bn = lmmp_limb_elem_mul_(&b, t, i);
            TEMP_B_FREE;
        } else {
            TEMP_B_DECL;
            mp_size_t tn = (n + 1) / 2 + 1;
            mp_limb_t* restrict t = BALLOC_TYPE(tn, mp_limb_t);
            mp_size_t i, j;
            for (i = 0, j = 0; j <= n; j += 2) {
                if (j + 1 <= n) {
                    t[i++] = (mp_limb_t)(x + j * m) * (x + (j + 1) * m);
                } else {
                    t[i++] = x + j * m;
                }
            }
            lmmp_debug_assert(i <= tn);
            bn = lmmp_limb_elem_mul_(&b, t, i);
            TEMP_B_FREE;
        }
        dst[shw + bn] = lmmp_shl_(dst + shw, b, bn, shl);
        rn = bn + shw + 1;
        rn -= dst[rn - 1] == 0 ? 1 : 0;
        lmmp_free(b);
        return rn;
    }
}