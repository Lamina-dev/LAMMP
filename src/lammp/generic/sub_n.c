/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of mylgpllib, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../../../include/lammp/lmmpn.h"

mp_limb_t lmmp_sub_nc_(mp_ptr dst, mp_srcptr numa, mp_srcptr numb, mp_size_t n, mp_limb_t c) {
    mp_size_t i;

    for (i = 0; i < n; i++) {
        mp_limb_t a, b;
        a = numa[i];
        b = numb[i];
        b += c;
        c = (b < c);
        c += (a < b);
        dst[i] = a - b;
    }
    return c;
}

mp_limb_t lmmp_sub_n_(mp_ptr dst, mp_srcptr numa, mp_srcptr numb, mp_size_t n) {
    mp_size_t i;
    mp_limb_t cy;

    for (i = 0, cy = 0; i < n; i++) {
        mp_limb_t a, b;
        a = numa[i];
        b = numb[i];
        b += cy;
        cy = (b < cy);
        cy += (a < b);
        dst[i] = a - b;
    }
    return cy;
}
