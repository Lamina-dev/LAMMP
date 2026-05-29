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

#ifdef _MSC_VER
#include <intrin.h>
#include <immintrin.h>
#elif defined(USE_ASM) && (defined(__x86_64__)) && (defined(__GNUC__) || defined(__clang__))
#include <x86intrin.h>
#endif

#include <stdint.h>

#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64))
// cnt = ctz(x)
// r = x >> cnt
// assume x is non-zero
#define ctz_shr_u64(r, x, cnt)           \
    do {                                 \
        unsigned long long _x_ = (x);    \
        unsigned long _bits_ = 0;        \
        _BitScanForward64(&_bits_, _x_); \
        cnt = _bits_;                    \
        (r) = _x_ >> (cnt);              \
    } while (0)
// cnt = clz(x)
// r = x << cnt
// assume x is non-zero
#define clz_shl_u64(r, x, cnt)                    \
    do {                                          \
        unsigned long long _x_ = (x);             \
        unsigned long _idx_;                      \
        _BitScanReverse64(&_idx_, _x_);           \
        (cnt) = 63 - (int)_idx_;                  \
        (r) = (unsigned long long)(_x_ << (cnt)); \
    } while (0)
#elif defined(__GNUC__) || defined(__clang__)
// cnt = ctz(x)
// r = x >> cnt
// assume x is non-zero
#define ctz_shr_u64(r, x, cnt)        \
    do {                              \
        unsigned long long _x_ = (x); \
        (cnt) = __builtin_ctzll(_x_); \
        (r) = _x_ >> (cnt);           \
    } while (0)
// cnt = clz(x)
// r = x << cnt
// assume x is non-zero
#define clz_shl_u64(r, x, cnt)                    \
    do {                                          \
        unsigned long long _x_ = (x);             \
        (cnt) = __builtin_clzll(_x_);             \
        (r) = (unsigned long long)(_x_ << (cnt)); \
    } while (0)
#else
// cnt = ctz(x)
// r = x >> cnt
// assume x is non-zero
#define ctz_shr_u64(r, x, cnt)   \
    do {                         \
        uint64_t _x_ = (x);      \
        int _i_ = 0;             \
        while ((_x_ & 1) == 0) { \
            _i_++;               \
            _x_ >>= 1;           \
        }                        \
        cnt = _i_;               \
        (r) = _x_;               \
    } while (0)
// cnt = clz(x)
// r = x << cnt
// assume x is non-zero
#define clz_shl_u64(r, x, cnt)                       \
    do {                                             \
        uint64_t _x_ = (x);                          \
        int _c_ = 0;                                 \
        while ((_x_ & 0x8000000000000000ULL) == 0) { \
            _c_++;                                   \
            _x_ <<= 1;                               \
        }                                            \
        (cnt) = _c_;                                 \
        (r) = _x_;                                   \
    } while (0)
#endif

#if defined(_MSC_VER)
// cnt = ctz(x)
// r = x >> cnt
// assume x is non-zero
#define ctz_shr_u32(r, x, cnt)         \
    do {                               \
        unsigned long _x_ = (x);       \
        unsigned long _bits_ = 0;      \
        _BitScanForward(&_bits_, _x_); \
        cnt = _bits_;                  \
        (r) = _x_ >> (cnt);            \
    } while (0)
// cnt = clz(x)
// r = x << cnt
// assume x is non-zero
#define clz_shl_u32(r, x, cnt)        \
    do {                              \
        unsigned long _x_ = (x);      \
        unsigned long _idx_;          \
        _BitScanReverse(&_idx_, _x_); \
        (cnt) = 31 - (int)_idx_;      \
        (r) = _x_ << (cnt);           \
    } while (0)
#elif defined(__GNUC__) || defined(__clang__)
// cnt = ctz(x)
// r = x >> cnt
// assume x is non-zero
#define ctz_shr_u32(r, x, cnt)      \
    do {                            \
        unsigned int _x_ = (x);     \
        (cnt) = __builtin_ctz(_x_); \
        (r) = _x_ >> (cnt);         \
    } while (0)
// cnt = clz(x)
// r = x << cnt
// assume x is non-zero
#define clz_shl_u32(r, x, cnt)      \
    do {                            \
        unsigned int _x_ = (x);     \
        (cnt) = __builtin_clz(_x_); \
        (r) = _x_ << (cnt);         \
    } while (0)
#else
// cnt = ctz(x)
// r = x >> cnt
// assume x is non-zero
#define ctz_shr_u32(r, x, cnt)    \
    do {                          \
        uint32_t _x_ = (x);       \
        int _i_ = 0;              \
        while ((_x_ & 1U) == 0) { \
            _i_++;                \
            _x_ >>= 1;            \
        }                         \
        cnt = _i_;                \
        (r) = _x_;                \
    } while (0)
// cnt = clz(x)
// r = x << cnt
// assume x is non-zero
#define clz_shl_u32(r, x, cnt)             \
    do {                                   \
        uint32_t _x_ = (x);                \
        int _c_ = 0;                       \
        while ((_x_ & 0x80000000U) == 0) { \
            _c_++;                         \
            _x_ <<= 1;                     \
        }                                  \
        (cnt) = _c_;                       \
        (r) = _x_;                         \
    } while (0)
#endif

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

static inline uint64_t _umul64to64hi_(uint64_t a, uint64_t b) {
#if (defined(__GNUC__) || defined(__clang__)) && defined(__SIZEOF_INT128__)
    __uint128_t t = (__uint128_t)a * (__uint128_t)b;
    return (uint64_t)(t >> 64);
#elif defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64))
    return __umulh(a, b);
#else
    uint64_t ah = a >> 32, bh = b >> 32;
    a = (uint32_t)a, b = (uint32_t)b;
    uint64_t r0 = a * b, r1 = a * bh, r2 = ah * b, r3 = ah * bh;
    r3 += (r1 >> 32) + (r2 >> 32);
    r1 = (uint32_t)r1, r2 = (uint32_t)r2;
    r1 += r2;
    r1 += (r0 >> 32);
    return r3 + (r1 >> 32);
#endif
}

static inline void _umulx64to128_(uint64_t a, uint64_t b, uint64_t* low, uint64_t* high) {
#if defined(USE_ASM) && (defined(__x86_64__))
    *low = _mulx_u64(a, b, high);
#else
    _umul64to128_(a, b, low, high);
#endif
}

static inline void _umul128to256_(uint64_t a_high, uint64_t a_low, uint64_t b_high, uint64_t b_low, uint64_t rr[4]) {
    uint64_t p1_low, p1_high;  // p1 = a_low × b_high
    uint64_t p2_low, p2_high;  // p2 = a_high × b_low
    _umulx64to128_(a_low, b_low, rr, rr + 1);
    _umulx64to128_(a_low, b_high, &p1_low, &p1_high);
    _umulx64to128_(a_high, b_low, &p2_low, &p2_high);
    _umulx64to128_(a_high, b_high, rr + 2, rr + 3);
    /*
        | res0 | res1 | res2 | res3 |
        |  p0l |  p0h |      |      |
               |  p1l |  p1h |      |
               |  p2l |  p2h |      |
               |      |  p3l |  p3h |
    */
    rr[1] += p1_low;
    uint64_t carry = (rr[1] < p1_low) ? 1 : 0;
    rr[1] += p2_low;
    carry += (rr[1] < p2_low) ? 1 : 0;

    rr[2] += carry;
    carry = (rr[2] < carry) ? 1 : 0;
    rr[2] += p1_high;
    carry += (rr[2] < p1_high) ? 1 : 0;
    rr[2] += p2_high;
    carry += (rr[2] < p2_high) ? 1 : 0;

    rr[3] += carry;
}

static inline void _umul128to128_(uint64_t a_high, uint64_t a_low, uint64_t b_high, uint64_t b_low, uint64_t rr[2]) {
    _umulx64to128_(a_low, b_low, rr, rr + 1);
    rr[1] += a_low * b_high;
    rr[1] += a_high * b_low;
}

static inline uint64_t _udiv128by64to64_(uint64_t numhi, uint64_t numlo, uint64_t den, uint64_t* r) {
#if defined(__GNUC__) || defined(__clang__)
#ifdef USE_ASM
    uint64_t result;
    __asm__("div %[v]" : "=a"(result), "=d"(*r) : [v] "r"(den), "a"(numlo), "d"(numhi));
    return result;
#else
    __uint128_t num = (__uint128_t)numhi << 64 | numlo;
    uint64_t result = num / den;
    *r = num % den;
    return result;
#endif
#else
    const uint64_t b = ((uint64_t)1 << 32);

    uint32_t q1;
    uint32_t q0;

    uint64_t q;

    int shift;

    uint64_t den10 = den;
    uint64_t num10 = numlo;

    uint32_t den1;
    uint32_t den0;
    uint32_t num1;
    uint32_t num0;

    uint64_t rem;

    uint64_t qhat;
    uint64_t rhat;

    uint64_t c1;
    uint64_t c2;

    if (numhi >= den) {
        if (r)
            *r = ~0ull;
        return ~0ull;
    }

    clz_shl_u64(den, den, shift);
    numhi <<= shift;
    numhi |= (numlo >> (-shift & 63)) & (uint64_t)(-(int64_t)shift >> 63);
    numlo <<= shift;

    num1 = (uint32_t)(numlo >> 32);
    num0 = (uint32_t)(numlo & 0xFFFFFFFFu);
    den1 = (uint32_t)(den >> 32);
    den0 = (uint32_t)(den & 0xFFFFFFFFu);

    qhat = numhi / den1;
    rhat = numhi % den1;
    c1 = qhat * den0;
    c2 = rhat * b + num1;
    if (c1 > c2)
        qhat -= (c1 - c2 > den) ? 2 : 1;
    q1 = (uint32_t)qhat;

    rem = numhi * b + num1 - q1 * den;

    qhat = rem / den1;
    rhat = rem % den1;
    c1 = qhat * den0;
    c2 = rhat * b + num0;
    if (c1 > c2)
        qhat -= (c1 - c2 > den) ? 2 : 1;
    q0 = (uint32_t)qhat;

    q = ((uint64_t)q1 << 32) | q0;

    if (r)
        *r = num10 - q * den10;
    return q;
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

// q = n0 / d0, assuming d0 is a 32-bit number, d0 > 1
// dinv = (B-1)//d0 + 1
#define _udiv32by32_q_preinv(q, n0, dinv)          \
    do {                                           \
        uint64_t _hi_, _lo_;                       \
        _umul64to128_((n0), (dinv), &_lo_, &_hi_); \
        (q) = _hi_;                                \
    } while (0)

/******************************
from https://libdivide.com/
******************************/

#define _U64_SHIFT_MASK 0x3F
#define _ADD_MARKER 0x40

typedef struct _udiv64_t {
    uint64_t magic;
    uint8_t more;
} _udiv64_t;

// assert d != 0
static inline _udiv64_t _udiv64_gen_internal_(uint64_t d, int branchfree) {
    _udiv64_t result;
    int shift;
    uint64_t t;
    clz_shl_u64(t, d, shift);
    uint32_t floor_log_2_d = 63 - shift;

    // Power of 2
    if ((d & (d - 1)) == 0) {
        // We need to subtract 1 from the shift value in case of an unsigned
        // branchfree divider because there is a hardcoded right shift by 1
        // in its division algorithm. Because of this we also need to add back
        // 1 in its recovery algorithm.
        result.magic = 0;
        result.more = (uint8_t)(floor_log_2_d - (branchfree != 0));
    } else {
        uint64_t proposed_m, rem;
        uint8_t more;
        // (1 << (64 + floor_log_2_d)) / d
        proposed_m = _udiv128by64to64_((uint64_t)1 << floor_log_2_d, 0, d, &rem);

        const uint64_t e = d - rem;

        // This power works if e < 2**floor_log_2_d.
        if (!branchfree && e < ((uint64_t)1 << floor_log_2_d)) {
            // This power works
            more = (uint8_t)floor_log_2_d;
        } else {
            // We have to use the general 65-bit algorithm.  We need to compute
            // (2**power) / d. However, we already have (2**(power-1))/d and
            // its remainder. By doubling both, and then correcting the
            // remainder, we can compute the larger division.
            // don't care about overflow here - in fact, we expect it
            proposed_m += proposed_m;
            const uint64_t twice_rem = rem + rem;
            if (twice_rem >= d || twice_rem < rem)
                proposed_m += 1;
            more = (uint8_t)(floor_log_2_d | _ADD_MARKER);
        }
        result.magic = 1 + proposed_m;
        result.more = more;
        // result.more's shift should in general be ceil_log_2_d. But if we
        // used the smaller power, we subtract one from the shift because we're
        // using the smaller power. If we're using the larger power, we
        // subtract one from the shift because it's taken care of by the add
        // indicator. So floor_log_2_d happens to be correct in both cases,
        // which is why we do it outside of the if statement.
    }
    return result;
}

// d > 1
static inline _udiv64_t _udiv64_gen(uint64_t d) {
    _udiv64_t tmp = _udiv64_gen_internal_(d, 1);
    _udiv64_t ret = {tmp.magic, (uint8_t)(tmp.more & _U64_SHIFT_MASK)};
    return ret;
}

static inline uint64_t _udiv64by64_q_preinv(uint64_t numer, const _udiv64_t* denom) {
    uint64_t q = _umul64to64hi_(numer, denom->magic);
    uint64_t t = ((numer - q) >> 1) + q;
    return t >> denom->more;
}

#endif // __LAMMP_LONGLONG_H__
