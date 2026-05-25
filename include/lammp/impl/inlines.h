/*
 * [LAMMP]
 * Copyright (C) [2025-2026] [HJimmyK(Jericho Knox)]
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __LAMMP_INLINES_H__
#define __LAMMP_INLINES_H__

#include "../lmmpn.h"
#include "mparam.h"

#ifdef _MSC_VER
#include <intrin.h>
#endif

static inline int __lmmp_limb_bits_(mp_limb_t x) {
    int k = 0;
    while (x) {
        x >>= 1;
        k++;
    }
    return k;
}

static inline int __lmmp_limb_popcnt_(mp_limb_t x) {
#ifdef __GNUC__
  return __builtin_popcountll(x);
#elif _MSC_VER
  return (int)__popcnt64(x);
#else
    int k = 0;
    while (x) {
        k += x & 1;
        x >>= 1;
    }
    return k;
#endif
}

static inline int __lmmp_leading_zeros_(mp_limb_t x) {
    if (x == 0) return 64;
#ifdef __GNUC__
    return __builtin_clzll(x);
#elif _MSC_VER
    unsigned long index;
    _BitScanReverse64(&index, x);  
    return 63 - (int)index;
#else
    int n = 0;
    if (x <= 0x00000000FFFFFFFF) { n += 32; x <<= 32; }
    if (x <= 0x0000FFFFFFFFFFFF) { n += 16; x <<= 16; }
    if (x <= 0x00FFFFFFFFFFFFFF) { n += 8;  x <<= 8; }
    if (x <= 0x0FFFFFFFFFFFFFFF) { n += 4;  x <<= 4; }
    if (x <= 0x3FFFFFFFFFFFFFFF) { n += 2;  x <<= 2; }
    if (x <= 0x7FFFFFFFFFFFFFFF) { n += 1;  x <<= 1; }
#endif
}

static inline int __lmmp_tailing_zeros_(mp_limb_t x) {
    if (x == 0) return 64;
#ifdef __GNUC__
    return __builtin_ctzll(x);
#elif _MSC_VER
    unsigned long index;
    _BitScanForward64(&index, x);
    return (int)index;
#else
    int n = 0;
    if ((x & 0x00000000FFFFFFFF) == 0) { n += 32; x >>= 32; }
    if ((x & 0x000000000000FFFF) == 0) { n += 16; x >>= 16; }
    if ((x & 0x00000000000000FF) == 0) { n += 8;  x >>= 8; }
    if ((x & 0x000000000000000F) == 0) { n += 4;  x >>= 4; }
    if ((x & 0x0000000000000003) == 0) { n += 2;  x >>= 2; }
    if ((x & 0x0000000000000001) == 0) { n += 1;  x >>= 1; }
#endif
}

static inline void __lmmp_mul_n_(mp_ptr dst, mp_srcptr numa, mp_srcptr numb, mp_size_t n) {
    if (n < MUL_TOOM22_THRESHOLD)
        lmmp_mul_basecase_(dst, numa, n, numb, n);
    else if (n < MUL_TOOM33_THRESHOLD)
        lmmp_mul_toom22_(dst, numa, n, numb, n);
    else if (n < MUL_TOOM44_THRESHOLD)
        lmmp_mul_toom33_(dst, numa, n, numb, n);
    else if (n < MUL_FFT_THRESHOLD)
        lmmp_mul_toom44_(dst, numa, n, numb, n);
    else
        lmmp_mul_fft_(dst, numa, n, numb, n);
}

static inline void __lmmp_sqr_(mp_ptr dst, mp_srcptr numa, mp_size_t na) {
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

#define lmmp_leading_zeros_ __lmmp_leading_zeros_
#define lmmp_tailing_zeros_ __lmmp_tailing_zeros_
#define lmmp_limb_bits_ __lmmp_limb_bits_
#define lmmp_limb_popcnt_ __lmmp_limb_popcnt_
#define lmmp_sqr_ __lmmp_sqr_
#define lmmp_mul_n_ __lmmp_mul_n_

#endif // __LAMMP_INLINES_H__