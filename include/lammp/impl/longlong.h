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

#ifndef __LAMMP_LONGLONG_H__
#define __LAMMP_LONGLONG_H__


#include <stdint.h>

static inline void _umul64to128_(uint64_t a, uint64_t b, uint64_t *low, uint64_t *high) {
#if (defined(__GNUC__) || defined(__clang__))
#if defined(USE_ASM) && (defined(__x86_64__))
    __asm__("mul %[b]" 
            : "=a"(*low),
              "=d"(*high)         
            : "a"(a), [b] "r"(b)  
            :                    
    );
#else
    __uint128_t prod = (__uint128_t)a * b;
    *low = (uint64_t)prod;
    *high = (uint64_t)(prod >> 64);
#endif
#elif defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64))
    *low = _umul128(a, b, high);
#else
    uint64_t ah = a >> 32, bh = b >> 32;
    a = (uint32_t)a, b = (uint32_t)b;
    uint64_t r0 = a * b, r1 = a * bh, r2 = ah * b, r3 = ah * bh;
    r3 += (r1 >> 32) + (r2 >> 32);
    r1 = (uint32_t)r1, r2 = (uint32_t)r2;
    r1 += r2;
    r1 += (r0 >> 32);
    *high = r3 + (r1 >> 32);
    *low = (r1 << 32) | (uint32_t)r0;
#endif
}

/**
 * 请注意，此处的蒙哥马利域的R为2^64，p不可超过2^63-1
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

#define _add_ssaaaa(sh, sl, ah, al, bh, bl) \
    do {                                    \
        uint64_t _x_;                       \
        _x_ = (al) + (bl);                  \
        (sh) = (ah) + (bh) + (_x_ < (al));  \
        (sl) = _x_;                         \
    } while (0)

#define _sub_ddmmss(sh, sl, ah, al, bh, bl) \
    do {                                    \
        uint64_t _x_;                       \
        _x_ = (al) - (bl);                  \
        (sh) = (ah) - (bh) - ((al) < (bl)); \
        (sl) = _x_;                         \
    } while (0)

#define _udiv_qrnnd_preinv(q, r, nh, nl, d, di)              \
    do {                                                     \
        uint64_t _qh_, _ql_, _r_, _mask_;                    \
        _umul64to128_((nh), (di), &_ql_, &_qh_);             \
        _add_ssaaaa(_qh_, _ql_, _qh_, _ql_, (nh) + 1, (nl)); \
        _r_ = (nl) - _qh_ * (d);                             \
        _mask_ = -(mp_limb_t)(_r_ > _ql_);                   \
        _qh_ += _mask_;                                      \
        _r_ += _mask_ & (d);                                 \
        if (_r_ >= (d)) {                                    \
            _r_ -= (d);                                      \
            _qh_++;                                          \
        }                                                    \
        (r) = _r_;                                           \
        (q) = _qh_;                                          \
    } while (0)

#define _udiv_qr_3by2(q, r1, r0, n2, n1, n0, d1, d0, dinv)                 \
    do {                                                                   \
        mp_limb_t _q0_, _t1_, _t0_, _mask_;                                \
        _umul64to128_((n2), (dinv), &_q0_, &(q));                          \
        _add_ssaaaa((q), _q0_, (q), _q0_, (n2), (n1));                     \
        /* Compute the two most significant limbs of n - q'd */            \
        (r1) = (n1) - (d1) * (q);                                          \
        _sub_ddmmss((r1), (r0), (r1), (n0), (d1), (d0));                   \
        _umul64to128_((d0), (q), &_t0_, &_t1_);                            \
        _sub_ddmmss((r1), (r0), (r1), (r0), _t1_, _t0_);                   \
        (q)++;                                                             \
        /* Conditionally adjust q and the remainders */                    \
        _mask_ = -(uint64_t)((r1) >= _q0_);                                \
        (q) += _mask_;                                                     \
        _add_ssaaaa((r1), (r0), (r1), (r0), _mask_ & (d1), _mask_ & (d0)); \
        if ((r1) >= (d1)) {                                                \
            if ((r1) > (d1) || (r0) >= (d0)) {                             \
                (q)++;                                                     \
                _sub_ddmmss((r1), (r0), (r1), (r0), (d1), (d0));           \
            }                                                              \
        }                                                                  \
    } while (0)

#endif // __LAMMP_LONGLONG_H__
