/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of mylgpllib, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../../../include/lammp/lmmpn.h"

void lmmp_not_(mp_ptr restrict dst, mp_srcptr restrict numa, mp_size_t na) {
    if (dst == numa) {
        for (mp_size_t i = 0; i < na; i++) {
            dst[i] = ~dst[i];
        }
    } else {
        for (mp_size_t i = 0; i < na; i++) {
            dst[i] = ~numa[i];
        }
    }
}

mp_limb_t lmmp_shlnot_(mp_ptr restrict dst, mp_srcptr restrict numa, mp_size_t na, mp_size_t shl) {
    if (shl == 0) {
        lmmp_not_(dst, numa, na);
        return 0;
    } else if (dst == numa) {
        mp_limb_t t, m, n;
        t = dst[0] << shl;
        m = dst[0] >> (LIMB_BITS - shl);
        dst[0] = ~t;
        for (mp_size_t i = 1; i < na; i++) {
            t = dst[i] << shl;
            n = dst[i] >> (LIMB_BITS - shl);
            dst[i] = ~(t | m);
            m = n;
        }
        return m;
    } else {
        /* seq(dst,numa) */
        mp_limb_t t, m, n;
        t = numa[0] << shl;
        m = numa[0] >> (LIMB_BITS - shl);
        dst[0] = ~t;
        for (mp_size_t i = 1; i < na; i++) {
            t = numa[i] << shl;
            n = numa[i] >> (LIMB_BITS - shl);
            dst[i] = ~(t | m);
            m = n;
        }
        return m;
    }
}