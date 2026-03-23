/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of mylgpllib, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../../../include/lammp/lmmpn.h"
#include "../../../include/lammp/numth.h"

static mp_size_t lmmp_1pow_1_(mp_ptr restrict dst) {
    dst[0] = 1;
    return 1;
}

static mp_size_t lmmp_2pow_1_(mp_ptr restrict dst, mp_size_t rn, ulong exp) {
    lmmp_zero(dst, exp / LIMB_BITS);
    dst[exp / LIMB_BITS] = (1ull << (exp % LIMB_BITS));
    rn = exp / LIMB_BITS + 1;
    return rn;
}

static mp_size_t lmmp_4pow_1_(mp_ptr restrict dst, mp_size_t rn, ulong exp) {
    exp <<= 1;
    lmmp_zero(dst, exp / LIMB_BITS);
    dst[exp / LIMB_BITS] = (1ull << (exp % LIMB_BITS));
    rn = exp / LIMB_BITS + 1;
    return rn;
}

static mp_size_t lmmp_8pow_1_(mp_ptr restrict dst, mp_size_t rn, ulong exp) {
    exp *= 3;
    lmmp_zero(dst, exp / LIMB_BITS);
    dst[exp / LIMB_BITS] = (1ull << (exp % LIMB_BITS));
    rn = exp / LIMB_BITS + 1;
    return rn;
}

static mp_size_t lmmp_3pow_1_(mp_ptr restrict dst, mp_size_t rn, ulong exp) {
    TEMP_DECL;
    static const mp_limb_t tab[32] = {
    1,           3,           9,            27,           81,            243,           729,            2187, 
    6561,        19683,       59049,        177147,       531441,        1594323,       4782969,        14348907, 
    43046721,    129140163,   387420489,    1162261467,   3486784401,    10460353203,   31381059609,    94143178827,
    282429536481,847288609443,2541865828329,7625597484987,22876792454961,68630377364883,205891132094649,617673396283947,
    };

    mp_ptr restrict sq = TALLOC_TYPE(rn, mp_limb_t);
    rn = 1;
    sq[0] = 1;
    int i = 12;
    while ((exp & (0x1full << ((i--) * 5))) == 0);
    for (++i; i >= 0; --i) {
        mp_size_t p = (exp & (0x1full << (i * 5))) >> (i * 5);
        dst[rn] = lmmp_mul_1_(dst, sq, rn, tab[p]);
        ++rn;
        rn -= (dst[rn - 1] == 0) ? 1 : 0;
        if (i > 0) {
            lmmp_sqr_(sq, dst, rn);
            rn <<= 1;
            rn -= (sq[rn - 1] == 0) ? 1 : 0;

            lmmp_sqr_(dst, sq, rn);
            rn <<= 1;
            rn -= (dst[rn - 1] == 0) ? 1 : 0;

            lmmp_sqr_(sq, dst, rn);
            rn <<= 1;
            rn -= (sq[rn - 1] == 0) ? 1 : 0;

            lmmp_sqr_(dst, sq, rn);
            rn <<= 1;
            rn -= (dst[rn - 1] == 0) ? 1 : 0;

            lmmp_sqr_(sq, dst, rn);
            rn <<= 1;
            rn -= (sq[rn - 1] == 0) ? 1 : 0;
        }
    }
    TEMP_FREE;
    return rn;
}

#define define_1_npow_1_(_n_)                                                                                    \
    static mp_size_t lmmp_##_n_##pow_1_(mp_ptr restrict dst, mp_size_t rn, ulong exp) {                          \
        TEMP_DECL;                                                                                               \
        static const mp_limb_t tab[16] = {                                                                       \
            1,                                                                                                   \
            (mp_limb_t)_n_,                                                                                      \
            (mp_limb_t)_n_ * _n_,                                                                                \
            (mp_limb_t)_n_ * _n_ * _n_,                                                                          \
            (mp_limb_t)_n_ * _n_ * _n_ * _n_,                                                                    \
            (mp_limb_t)_n_ * _n_ * _n_ * _n_ * _n_,                                                              \
            (mp_limb_t)_n_ * _n_ * _n_ * _n_ * _n_ * _n_,                                                        \
            (mp_limb_t)_n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_,                                                  \
            (mp_limb_t)_n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_,                                            \
            (mp_limb_t)_n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_,                                      \
            (mp_limb_t)_n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_,                                \
            (mp_limb_t)_n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_,                          \
            (mp_limb_t)_n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_,                    \
            (mp_limb_t)_n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_,              \
            (mp_limb_t)_n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_,        \
            (mp_limb_t)_n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_ * _n_}; \
                                                                                                                 \
        mp_size_t sqn = (rn + 2) >> 1;                                                                           \
        mp_ptr restrict sq = TALLOC_TYPE(sqn, mp_limb_t);                                                        \
        rn = 1;                                                                                                  \
        dst[0] = 1;                                                                                              \
        int i = 15;                                                                                              \
        while ((exp & (0xfull << ((i--) * 4))) == 0);                                                            \
        for (++i; i >= 0; --i) {                                                                                 \
            mp_size_t p = (exp & (0xfull << (i * 4))) >> (i * 4);                                                \
            dst[rn] = lmmp_mul_1_(dst, dst, rn, tab[p]);                                                         \
            ++rn;                                                                                                \
            rn -= (dst[rn - 1] == 0) ? 1 : 0;                                                                    \
            if (i > 0) {                                                                                         \
                lmmp_sqr_(sq, dst, rn);                                                                          \
                rn <<= 1;                                                                                        \
                rn -= (sq[rn - 1] == 0) ? 1 : 0;                                                                 \
                                                                                                                 \
                lmmp_sqr_(dst, sq, rn);                                                                          \
                rn <<= 1;                                                                                        \
                rn -= (dst[rn - 1] == 0) ? 1 : 0;                                                                \
                                                                                                                 \
                lmmp_sqr_(sq, dst, rn);                                                                          \
                rn <<= 1;                                                                                        \
                rn -= (sq[rn - 1] == 0) ? 1 : 0;                                                                 \
                                                                                                                 \
                lmmp_sqr_(dst, sq, rn);                                                                          \
                rn <<= 1;                                                                                        \
                rn -= (dst[rn - 1] == 0) ? 1 : 0;                                                                \
            }                                                                                                    \
        }                                                                                                        \
        TEMP_FREE;                                                                                               \
        return rn;                                                                                               \
    }

define_1_npow_1_(5)
define_1_npow_1_(7)
define_1_npow_1_(11)
define_1_npow_1_(13)
define_1_npow_1_(15)

static mp_size_t lmmp_6pow_1_(mp_ptr restrict dst, mp_size_t rn, ulong exp) {
    mp_size_t n = exp / LIMB_BITS;
    lmmp_zero(dst, n);
    rn = lmmp_3pow_1_(dst + n, rn - n, exp);
    dst[n + rn] = lmmp_shl_(dst + n, dst + n, rn, exp % LIMB_BITS);
    rn += n;
    rn -= dst[rn - 1] == 0 ? 1 : 0;
    return rn;
}

static mp_size_t lmmp_9pow_1_(mp_ptr restrict dst, mp_size_t rn, ulong exp) {
    TEMP_DECL;
    mp_ptr restrict tp = TALLOC_TYPE((rn + 1) >> 1, mp_limb_t);
    rn = lmmp_3pow_1_(tp, (rn + 1) >> 1, exp);
    lmmp_sqr_(dst, tp, rn);
    rn <<= 1;
    rn -= dst[rn - 1] == 0 ? 1 : 0;
    TEMP_FREE;
    return rn;
}

static mp_size_t lmmp_10pow_1_(mp_ptr restrict dst, mp_size_t rn, ulong exp) {
    mp_size_t n = exp / LIMB_BITS;
    lmmp_zero(dst, n);
    rn = lmmp_5pow_1_(dst + n, rn - n, exp);
    dst[n + rn] = lmmp_shl_(dst + n, dst + n, rn, exp % LIMB_BITS);
    rn += n + 1;
    rn -= dst[rn - 1] == 0 ? 1 : 0;
    return rn;
}

static mp_size_t lmmp_12pow_1_(mp_ptr restrict dst, mp_size_t rn, ulong exp) {
    mp_size_t n = (2 * exp) / LIMB_BITS;
    lmmp_zero(dst, n);
    lmmp_assert(rn > n);
    rn = lmmp_3pow_1_(dst + n, rn - n, exp);
    dst[n + rn] = lmmp_shl_(dst + n, dst + n, rn, (2 * exp) % LIMB_BITS);
    rn += n + 1;
    rn -= dst[rn - 1] == 0 ? 1 : 0;
    return rn;
}

static mp_size_t lmmp_14pow_1_(mp_ptr restrict dst, mp_size_t rn, ulong exp) {
    mp_size_t n = exp / LIMB_BITS;
    lmmp_zero(dst, n);
    rn = lmmp_7pow_1_(dst + n, rn - n, exp);
    dst[n + rn] = lmmp_shl_(dst + n, dst + n, rn, exp % LIMB_BITS);
    rn += n + 1;
    rn -= dst[rn - 1] == 0 ? 1 : 0;
    return rn;
}

mp_size_t lmmp_1_pow_1_(mp_ptr restrict dst, mp_size_t rn, ulong base, ulong exp) {
    switch (base) {
        case 1:
            return lmmp_1pow_1_(dst);
        case 2:
            return lmmp_2pow_1_(dst, rn, exp);
        case 4:
            return lmmp_4pow_1_(dst, rn, exp);
        case 8:
            return lmmp_8pow_1_(dst, rn, exp);
        case 3:
            return lmmp_3pow_1_(dst, rn, exp);
        case 5:
            return lmmp_5pow_1_(dst, rn, exp);
        case 7:
            return lmmp_7pow_1_(dst, rn, exp);
        case 11:
            return lmmp_11pow_1_(dst, rn, exp);
        case 13:
            return lmmp_13pow_1_(dst, rn, exp);
        case 15:
            return lmmp_15pow_1_(dst, rn, exp);
        case 6:
            return lmmp_6pow_1_(dst, rn, exp);
        case 9:
            return lmmp_9pow_1_(dst, rn, exp);
        case 10:
            return lmmp_10pow_1_(dst, rn, exp);
        case 12:
            return lmmp_12pow_1_(dst, rn, exp);
        case 14:
            return lmmp_14pow_1_(dst, rn, exp);
        default:
            lmmp_assert(base <= 15 && base >= 1);
            return 0;
    }
}

#define cal_tab(i)                  \
    tab[i] = tab[i - 1] * base;     \
    tab[i + 1] = tab[i] * base;     \
    tab[i + 2] = tab[i + 1] * base; \
    tab[i + 3] = tab[i + 2] * base; \
    tab[i + 4] = tab[i + 3] * base; \
    tab[i + 5] = tab[i + 4] * base; \
    tab[i + 6] = tab[i + 5] * base

mp_size_t lmmp_2_pow_1_(mp_ptr restrict dst, mp_size_t rn, ulong base, ulong exp) {
    lmmp_param_assert(base >= 2);
    lmmp_param_assert(base <= 0xff);
    TEMP_DECL;
    mp_limb_t tab[8];
    tab[0] = 1;
    cal_tab(1);

    mp_ptr restrict sq = TALLOC_TYPE(rn, mp_limb_t);
    sq[0] = 1;
    rn = 1;
    int i = 21;
    while ((exp & (0x7ull << ((i--) * 3))) == 0);
    for (++i; i >= 0; --i) {
        mp_size_t p = (exp & (0x7ull << (i * 3))) >> (i * 3);
        dst[rn] = lmmp_mul_1_(dst, sq, rn, tab[p]);
        ++rn;
        rn -= (dst[rn - 1] == 0) ? 1 : 0;
        if (i > 0) {
            lmmp_sqr_(sq, dst, rn);
            rn <<= 1;
            rn -= (sq[rn - 1] == 0) ? 1 : 0;
            lmmp_sqr_(dst, sq, rn);
            rn <<= 1;
            rn -= (dst[rn - 1] == 0) ? 1 : 0;
            lmmp_sqr_(sq, dst, rn);
            rn <<= 1;
            rn -= (sq[rn - 1] == 0) ? 1 : 0;
        }
    }
    TEMP_FREE;
    return rn;
}

mp_size_t lmmp_4_pow_1_(mp_ptr restrict dst, mp_size_t rn, ulong base, ulong exp) {
    lmmp_param_assert(base >= 2);
    lmmp_param_assert(base <= 0xffff);
    TEMP_DECL;
    mp_limb_t tab[8][2];
    tab[0][0] = 1;
    tab[0][1] = 0;
    tab[1][0] = base;
    tab[1][1] = 0;
    tab[2][0] = tab[1][0] * base;
    tab[2][1] = 0;
    tab[3][0] = tab[2][0] * base;
    tab[3][1] = 0;
    lmmp_mullh_(tab[2][0], tab[2][0], tab[4]);
    lmmp_mullh_(tab[3][0], tab[2][0], tab[5]);
    lmmp_mullh_(tab[3][0], tab[3][0], tab[6]);
    lmmp_mullh_(tab[3][0], tab[4][0], tab[7]);
    tab[7][1] += tab[3][0] * tab[4][1];

    mp_ptr restrict sq = TALLOC_TYPE(rn, mp_limb_t);
    rn = 1;
    dst[0] = 1;
    int i = 21;
    while ((exp & (0x7ull << ((i--) * 3))) == 0);
    for (++i; i >= 0; --i) {
        mp_size_t p = (exp & (0x7ull << (i * 3))) >> (i * 3);
        mp_srcptr tap = tab[p];
        mp_size_t tan = (tap[1] != 0) ? 2 : 1;
        if (rn >= tan)
            lmmp_mul_basecase_(sq, dst, rn, tap, tan);
        else 
            lmmp_mul_basecase_(sq, tap, tan, dst, rn);
        rn += tan;
        rn -= (sq[rn - 1] == 0) ? 1 : 0;

        if (i > 0) {
            lmmp_sqr_(dst, sq, rn);
            rn <<= 1;
            rn -= (dst[rn - 1] == 0) ? 1 : 0;
            lmmp_sqr_(sq, dst, rn);
            rn <<= 1;
            rn -= (sq[rn - 1] == 0) ? 1 : 0;
            lmmp_sqr_(dst, sq, rn);
            rn <<= 1;
            rn -= (dst[rn - 1] == 0) ? 1 : 0;
        }
    }
    lmmp_copy(dst, sq, rn);
    TEMP_FREE;
    return rn;
}

mp_size_t lmmp_8_pow_1_(mp_ptr restrict dst, mp_size_t rn, ulong base, ulong exp) {
    lmmp_param_assert(base >= 2);
    lmmp_param_assert(base <= 0xffffffff);
    TEMP_DECL;
    mp_limb_t tab[4][2];
    tab[0][0] = 1;
    tab[0][1] = 0;
    tab[1][0] = base;
    tab[1][1] = 0;
    tab[2][0] = tab[1][0] * base;
    tab[2][1] = 0;
    lmmp_mullh_(tab[2][0], base, tab[3]);
    tab[3][1] += tab[2][1] * base;

    bool rsq = true; /* is true mean now sq is result */
    mp_ptr restrict sq = TALLOC_TYPE(rn, mp_limb_t);
    sq[0] = 1;
    rn = 1;
    int i = 31;
    while ((exp & (0x3ull << ((i--) * 2))) == 0);
    for (++i; i >= 0; --i) {
        mp_size_t p = (exp & (0x3ull << (i * 2))) >> (i * 2);
        mp_srcptr tap = tab[p];
        mp_size_t tan = (tap[1] != 0) ? 2 : 1;
        if (rsq) {
            if (tan >= rn)
                lmmp_mul_basecase_(dst, tap, tan, sq, rn);
            else
                lmmp_mul_basecase_(dst, sq, rn, tap, tan);
            rn += tan;
            rn -= (dst[rn - 1] == 0) ? 1 : 0;
            rsq = false;
        } else {
            lmmp_mul_basecase_(sq, dst, rn, tap, tan);
            rn += tan;
            rn -= (sq[rn - 1] == 0) ? 1 : 0;
            rsq = true;
        }

        if (i > 0) {
            if (rsq) {
                lmmp_sqr_(dst, sq, rn);
                rn <<= 1;
                rn -= (dst[rn - 1] == 0) ? 1 : 0;
                lmmp_sqr_(sq, dst, rn);
                rn <<= 1;
                rn -= (sq[rn - 1] == 0) ? 1 : 0;
            } else {
                lmmp_sqr_(sq, dst, rn);
                rn <<= 1;
                rn -= (sq[rn - 1] == 0) ? 1 : 0;
                lmmp_sqr_(dst, sq, rn);
                rn <<= 1;
                rn -= (dst[rn - 1] == 0) ? 1 : 0;
            }
        }
    }
    if (rsq)
        lmmp_copy(dst, sq, rn);
    TEMP_FREE;
    return rn;
}

mp_size_t lmmp_16_pow_1_(mp_ptr restrict dst, mp_size_t rn, ulong base, ulong exp) {
    lmmp_param_assert(base > 0xffffffff);
    TEMP_DECL;

    mp_limb_t b2[2];
    lmmp_mullh_(base, base, b2);
    mp_limb_t b3[3];
    b3[2] = lmmp_mul_1_(b3, b2, 2, base);
    mp_size_t b3n = b3[2] != 0 ? 3 : 2;

    bool rsq = true; /* is true mean now sq is result */
    mp_ptr restrict sq = TALLOC_TYPE(rn, mp_limb_t);
    rn = 1;
    sq[0] = 1;
    int i = 31;
    while ((exp & (0x3ull << ((i--) * 2))) == 0);
    for (++i; i >= 0; --i) {
        mp_size_t p = (exp & (0x3ull << (i * 2))) >> (i * 2);
        if (p == 0) {
            ;
        } else if (p == 1) {
            if (rsq) {
                sq[rn] = lmmp_mul_1_(sq, sq, rn, base);
                ++rn;
                rn -= (sq[rn - 1] == 0) ? 1 : 0;
            } else {
                dst[rn] = lmmp_mul_1_(dst, dst, rn, base);
                ++rn;
                rn -= (dst[rn - 1] == 0) ? 1 : 0;
            }
        } else if (p == 2) {
            if (rsq) {
                if (2 >= rn)
                    lmmp_mul_basecase_(dst, b2, 2, sq, rn);
                else
                    lmmp_mul_basecase_(dst, sq, rn, b2, 2);
                rn += 2;
                rn -= (dst[rn - 1] == 0) ? 1 : 0;
                rsq = false;
            } else {
                lmmp_mul_basecase_(sq, dst, rn, b2, 2);
                rn += 2;
                rn -= (sq[rn - 1] == 0) ? 1 : 0;
                rsq = true;
            }
        } else {
            if (rsq) {
                if (b3n >= rn)
                    lmmp_mul_basecase_(dst, b3, b3n, sq, rn);
                else
                    lmmp_mul_basecase_(dst, sq, rn, b3, b3n);
                rn += b3n;
                rn -= (dst[rn - 1] == 0) ? 1 : 0;
                rsq = false;
            } else {
                lmmp_mul_basecase_(sq, dst, rn, b3, b3n);
                rn += b3n;
                rn -= (sq[rn - 1] == 0) ? 1 : 0;
                rsq = true;
            }
        }
        
        if (i > 0) {
            if (rsq) {
                lmmp_sqr_(dst, sq, rn);
                rn <<= 1;
                rn -= (dst[rn - 1] == 0) ? 1 : 0;
                lmmp_sqr_(sq, dst, rn);
                rn <<= 1;
                rn -= (sq[rn - 1] == 0) ? 1 : 0;
            } else {
                lmmp_sqr_(sq, dst, rn);
                rn <<= 1;
                rn -= (sq[rn - 1] == 0) ? 1 : 0;
                lmmp_sqr_(dst, sq, rn);
                rn <<= 1;
                rn -= (dst[rn - 1] == 0) ? 1 : 0;
            }
        }
    }
    if (rsq)
        lmmp_copy(dst, sq, rn);
    TEMP_FREE;
    return rn;
}

mp_size_t lmmp_pow_1_(mp_ptr restrict dst, mp_size_t rn, mp_limb_t base, ulong exp) {
    lmmp_param_assert(base > 1);
    lmmp_param_assert(exp > 0);
    if (base <= (mp_limb_t)0xf) {
        return lmmp_1_pow_1_(dst, rn, base, exp);
    } else {
        int tz = lmmp_tailing_zeros_(base);
        base >>= tz;
        mp_size_t shw = (exp * tz) / LIMB_BITS;
        int shl = (exp * tz) % LIMB_BITS;

        lmmp_zero(dst, shw);
        if (base <= (mp_limb_t)0xf)
            rn = lmmp_1_pow_1_(dst + shw, rn - shw, base, exp);
        else if (base <= (mp_limb_t)0xff)
            rn = lmmp_2_pow_1_(dst + shw, rn - shw, base, exp);
        else if (base <= (mp_limb_t)0xffff)
            rn = lmmp_4_pow_1_(dst + shw, rn - shw, base, exp);
        else if (base <= (mp_limb_t)0xffffffff)
            rn = lmmp_8_pow_1_(dst + shw, rn - shw, base, exp);
        else
            rn = lmmp_16_pow_1_(dst + shw, rn - shw, base, exp);
        if (shl) {
            dst[shw + rn] = lmmp_shl_(dst + shw, dst + shw, rn, shl);
            rn += shw + 1;
            rn -= (dst[rn - 1] == 0) ? 1 : 0;
        } else {
            rn += shw;
        }
        return rn;
    }
}
