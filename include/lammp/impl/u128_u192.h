/*
===============================================================================
1. LGPL v2.1 LICENSED CODE (LAMMP Project)
Copyright (c) 2025-2026 HJimmyK/LAMINA
This file is part of LAMMP (LGPL v2.1 License)
Full LGPL v2.1 License Text: https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
LAMMP Repository: https://github.com/Lamina-dev/LAMMP

Modification Note: This file contains modifications to the original MIT-licensed code to adapt to LAMMP's LGPL v2.1
environment.

===============================================================================
2. MIT LICENSED CODE (Original Source)
MIT License

Copyright (c) 2024-2050 HJimmyK
Project URL: https://github.com/HJimmyK/fast_mul_3ntt_crt_CC

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef __LAMMP_U128_U192_H__
#define __LAMMP_U128_U192_H__

#include "_umul64to128_.h"
/**
 * 请注意，本文中的蒙哥马利域的R为2^64，p不可超过2^63-1
 */

typedef uint64_t u128[2];
typedef uint64_t u192[3];

#define _u128lshl(x, y, n)                                             \
    do {                                                               \
        (*((x) + 1)) = ((*(y)) >> (64 - (n))) | ((*((y) + 1)) << (n)); \
        (*(x)) = (*(y)) << (n);                                        \
    } while (0)

#define _u128lshr(x, y, n)                                       \
    do {                                                         \
        (*(x)) = ((*(y)) >> (n)) | ((*((y) + 1)) << (64 - (n))); \
        (*((x) + 1)) = (*((y) + 1)) >> (n);                      \
    } while (0)

#define _u128high(x) (*((x) + 1))

#define _u128low(x) (*(x))

#define _u128add(r, x, y)                                                       \
    do {                                                                        \
        (*(r)) = *(x) + *(y);                                                   \
        (*((r) + 1)) = (*((x) + 1)) + (*((y) + 1)) + ((*(r)) < (*(y)) ? 1 : 0); \
    } while (0)

#define _u128add64(r, x, _i64)                                     \
    do {                                                           \
        (*(r)) = *(x) + (_i64);                                    \
        (*((r) + 1)) = (*((x) + 1)) + (((*(r)) < (_i64)) ? 1 : 0); \
    } while (0)

#define _u128sub64(r, x, _i64)          \
    do {                                \
        uint64_t _c_ = (x)[0] < (_i64); \
        (r)[0] = (x)[0] - (_i64);       \
        (r)[1] = (x)[1] - _c_;          \
    } while (0)

// x < y ?
#define _u128cmp(x, y) ((x)[1] < (y)[1] || ((x)[1] == (y)[1] && (x)[0] < (y)[0]))

#define _u128sub(r, x, y)               \
    do {                                \
        uint64_t _c_ = (x)[0] < (y)[0]; \
        (r)[0] = (x)[0] - (y)[0];       \
        (r)[1] = (x)[1] - (y)[1] - _c_; \
    } while (0)

#define _u128mul(r, x, y) _umul64to128_((x), (y), (r), (((r) + 1)))

#define _mont64_add(r, x, y, mod2)                   \
    do {                                             \
        (r) = (x) + (y);                             \
        (r) = ((r) < (mod2)) ? (r) : ((r) - (mod2)); \
    } while (0)

#define _mont64_sub(r, x, y, mod2)                                        \
    do {                                                                  \
        (r) = (((x) - (y)) > (x)) ? (((x) - (y)) + (mod2)) : ((x) - (y)); \
    } while (0)

#define _raw64_add(r, x, y) \
    do {                    \
        (r) = (x) + (y);    \
    } while (0)

#define _raw64_sub(r, x, y, mod2) \
    do {                          \
        (r) = (x) - (y) + (mod2); \
    } while (0)

#define _mont64_norm2(r, x, mod2)                   \
    do {                                            \
        (r) = (x) >= (mod2) ? ((x) - (mod2)) : (x); \
    } while (0)

/*
 * macro _mont_mul(r, x, y, mod, modInvNeg) whithout "p_mont = t if t < p else t - p"
 * macro _mont_mulinto(x, y, mod, modInvNeg) with "p_mont = t if t < p else t - p"
 *
 * assert((x < mod) && (y < mod))
 *
 */
#define _mont64_mul(r, x, y, mod, modInvNeg)   \
    do {                                       \
        u128 _tmp1_ = {0, 0};                  \
        _u128mul(_tmp1_, (x), (y));            \
        u128 _tmp2_ = {0, 0};                  \
        (*_tmp2_) = (*(_tmp1_)) * (modInvNeg); \
        _u128mul(_tmp2_, (*(_tmp2_)), (mod));  \
        _u128add(_tmp2_, _tmp2_, _tmp1_);      \
        (r) = (*(_tmp2_ + 1));                 \
    } while (0)

/*
def montgomery_mul(a_mont, b_mont, p):
    R = 1 << 64
    p_inv_mod_R = pow(p, -1, R)
    ab = a_mont * b_mont
    m = (ab * p_inv_mod_R) % R
    t = (ab + m * p) // R
    p_mont = t if t < p else t - p
    return p_mont
*/

#define _mont64_mulinto(x, y, mod, modInvNeg)                                        \
    do {                                                                             \
        u128 _tmp1_ = {0, 0};                                                        \
        _u128mul(_tmp1_, (x), (y));                                                  \
        u128 _tmp2_ = {0, 0};                                                        \
        *(_tmp2_) = (*(_tmp1_)) * (modInvNeg);                                       \
        _u128mul(_tmp2_, *(_tmp2_), (mod));                                          \
        _u128add(_tmp2_, _tmp2_, _tmp1_);                                            \
        (x) = (*(_tmp2_ + 1) < (mod)) ? (*(_tmp2_ + 1)) : ((*(_tmp2_ + 1)) - (mod)); \
    } while (0)

#define _mont64_tomont(x, r2, mod, modInvNeg)                                          \
    do {                                                                               \
        u128 _tmp1_ = {0, 0};                                                          \
        _u128mul(_tmp1_, (x), (r2));                                                   \
        u128 _tmp2_ = {0, 0};                                                          \
        *(_tmp2_) = (*(_tmp1_)) * (modInvNeg);                                         \
        _u128mul(_tmp2_, (*(_tmp2_)), (mod));                                          \
        _u128add(_tmp2_, _tmp2_, _tmp1_);                                              \
        (x) = ((*(_tmp2_ + 1)) < (mod)) ? (*(_tmp2_ + 1)) : ((*(_tmp2_ + 1)) - (mod)); \
    } while (0)

#define _mont64_toint(x, mod, modInvneg)                                          \
    do {                                                                          \
        u128 _tmp_ = {0, 0};                                                      \
        *_tmp_ = (x) * (modInvneg);                                               \
        _u128mul(_tmp_, (*_tmp_), (mod));                                         \
        _u128add64(_tmp_, _tmp_, x);                                              \
        (x) = (*(_tmp_ + 1) < (mod)) ? (*(_tmp_ + 1)) : ((*(_tmp_ + 1)) - (mod)); \
    } while (0)

#define _u128x64to192(i192, i128, i64)                                 \
    do {                                                               \
        _umul64to128_((*(i128)), i64, (i192), ((i192) + 1));           \
        uint64_t _tmp_;                                                \
        _umul64to128_((*((i128) + 1)), i64, (&(_tmp_)), ((i192) + 2)); \
        (*((i192) + 1)) += _tmp_;                                      \
        (*((i192) + 2)) += ((*((i192) + 1)) < _tmp_) ? 1 : 0;          \
    } while (0)

#define _u192add(i192, j192)                                \
    do {                                                    \
        (*(i192)) += (*(j192));                             \
        uint64_t _c_ = ((*(i192)) < (*j192)) ? 1 : 0;       \
        (*((i192) + 1)) += _c_;                             \
        _c_ = ((*((i192) + 1)) < _c_) ? 1 : 0;              \
        (*((i192) + 1)) += (*((j192) + 1));                 \
        _c_ += ((*((i192) + 1)) < (*((j192) + 1))) ? 1 : 0; \
        (*((i192) + 2)) += _c_ + (*((j192) + 2));           \
    } while (0)

#define _u192sub(i192, j192)                             \
    do {                                                 \
        uint64_t _b_ = ((i192)[0] < (j192)[0]) ? 1 : 0;  \
        (i192)[0] -= (j192)[0];                          \
        uint64_t _b1_ = ((i192)[1] < (j192)[1]) ? 1 : 0; \
        (i192)[1] -= (j192)[1];                          \
        _b1_ += ((i192)[1] < _b_) ? 1 : 0;               \
        (i192)[1] -= _b_;                                \
        (i192)[2] = (i192)[2] - ((j192)[2] + _b1_);      \
    } while (0)

#endif  // __LAMMP_U128_U192_H__