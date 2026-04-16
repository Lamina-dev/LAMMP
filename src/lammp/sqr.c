/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../../include/lammp/impl/mparam.h"
#include "../../include/lammp/lmmpn.h"

void lmmp_sqr_(mp_ptr dst, mp_srcptr numa, mp_size_t na) {
    if (na < MUL_TOOM22_THRESHOLD)
        lmmp_sqr_basecase_(dst, numa, na);
    else if (na < MUL_TOOM33_THRESHOLD)
        lmmp_sqr_toom2_(dst, numa, na);
    else if (na < MUL_TOOM44_THRESHOLD)
        lmmp_sqr_toom3_(dst, numa, na);
    else if (na < MUL_FFT_THRESHOLD)
        lmmp_sqr_toom4_(dst, numa, na);
    else
        lmmp_mul_fft_(dst, numa, na, numa, na);
}