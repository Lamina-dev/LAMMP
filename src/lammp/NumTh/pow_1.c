#include "../../../include/lammp/numth.h"
#include <math.h>

#define cal_tab(i)                  \
    tab[i] = tab[i - 1] * base;     \
    tab[i + 1] = tab[i] * base;     \
    tab[i + 2] = tab[i + 1] * base; \
    tab[i + 3] = tab[i + 2] * base; \
    tab[i + 4] = tab[i + 3] * base; \
    tab[i + 5] = tab[i + 4] * base; \
    tab[i + 6] = tab[i + 5] * base

#define cal_p(i, w)                       \
    p[i] = exp & ((1ull << (w)) - 1);     \
    exp >>= w;                            \
    p[i + 1] = exp & ((1ull << (w)) - 1); \
    exp >>= w;                            \
    p[i + 2] = exp & ((1ull << (w)) - 1); \
    exp >>= w;                            \
    p[i + 3] = exp & ((1ull << (w)) - 1); \
    exp >>= w;                            \
    p[i + 4] = exp & ((1ull << (w)) - 1); \
    exp >>= w

mp_size_t lmmp_1_pow_1_(mp_ptr dst, mp_size_t rn, mp_limb_t base, mp_limb_t exp) {
    mp_limb_t tab[16];
    tab[0] = 1;
    tab[1] = base;
    cal_tab(2);
    cal_tab(9);

    mp_byte_t p[16];
    cal_p(0, 4);
    cal_p(5, 4);
    cal_p(10, 4);
    p[15] = exp & 0xf;

    TEMP_DECL;
    mp_size_t dsn = 1;
    mp_ptr sq = TALLOC_TYPE(rn, mp_limb_t);
    sq[0] = 1;
    mp_size_t i = 15;
    while (p[i--] == 0);
    for (++i; i != (mp_size_t)-1; i--) {
        sq[dsn] = lmmp_mul_1_(sq, sq, dsn, tab[p[i]]);
        ++dsn;
        dsn -= (sq[dsn - 1] == 0) ? 1 : 0;
        if (i > 0) {
            lmmp_sqr_(dst, sq, dsn);
            dsn <<= 1;
            dsn -= (dst[dsn - 1] == 0) ? 1 : 0;
            lmmp_sqr_(sq, dst, dsn);
            dsn <<= 1;
            dsn -= (sq[dsn - 1] == 0) ? 1 : 0;
            lmmp_sqr_(dst, sq, dsn);
            dsn <<= 1;
            dsn -= (dst[dsn - 1] == 0) ? 1 : 0;
            lmmp_sqr_(sq, dst, dsn);
            dsn <<= 1;
            dsn -= (sq[dsn - 1] == 0) ? 1 : 0;
        }
    }
    lmmp_copy(dst, sq, dsn);
    TEMP_FREE;
    return dsn;
}

mp_size_t lmmp_2_pow_1_(mp_ptr dst, mp_size_t rn, mp_limb_t base, mp_limb_t exp) {
    mp_limb_t tab[8];
    tab[0] = 1;
    cal_tab(1);

    mp_byte_t p[22];
    cal_p(0, 3);
    cal_p(5, 3);
    cal_p(10, 3);
    cal_p(15, 3);
    p[20] = exp & 0x7;
    exp >>= 3;
    p[21] = exp & 0x7;

    TEMP_DECL;
    mp_size_t dsn = 1;
    bool rsq = true; /* is true mean now sq is result */
    mp_ptr sq = TALLOC_TYPE(rn, mp_limb_t);
    sq[0] = 1;
    mp_size_t i = 21;
    while (p[i--] == 0);
    for (++i; i != (mp_size_t)-1; i--) {
        if (rsq) {
            sq[dsn] = lmmp_mul_1_(sq, sq, dsn, tab[p[i]]);
            ++dsn;
            dsn -= (sq[dsn - 1] == 0) ? 1 : 0;
        } else {
            dst[dsn] = lmmp_mul_1_(dst, dst, dsn, tab[p[i]]);
            ++dsn;
            dsn -= (dst[dsn - 1] == 0) ? 1 : 0;
        }

        if (i > 0) {
            if (rsq) {
                lmmp_sqr_(dst, sq, dsn);
                dsn <<= 1;
                dsn -= (dst[dsn - 1] == 0) ? 1 : 0;
                lmmp_sqr_(sq, dst, dsn);
                dsn <<= 1;
                dsn -= (sq[dsn - 1] == 0) ? 1 : 0;
                lmmp_sqr_(dst, sq, dsn);
                dsn <<= 1;
                dsn -= (dst[dsn - 1] == 0) ? 1 : 0;
                rsq = false;
            } else {
                lmmp_sqr_(sq, dst, dsn);
                dsn <<= 1;
                dsn -= (sq[dsn - 1] == 0) ? 1 : 0;
                lmmp_sqr_(dst, sq, dsn);
                dsn <<= 1;
                dsn -= (dst[dsn - 1] == 0) ? 1 : 0;
                lmmp_sqr_(sq, dst, dsn);
                dsn <<= 1;
                dsn -= (sq[dsn - 1] == 0) ? 1 : 0;
                rsq = true;
            }
        }
    }
    if (rsq)
        lmmp_copy(dst, sq, dsn);
    TEMP_FREE;
    return dsn;
}

mp_size_t lmmp_4_pow_1_(mp_ptr dst, mp_size_t rn, mp_limb_t base, mp_limb_t exp) {
    mp_limb_t tab[8][2];
    tab[0][0] = 1;
    tab[0][1] = 0;
    tab[1][0] = base;
    tab[1][1] = 0;
    tab[2][0] = base * base;
    tab[2][1] = 0;
    tab[3][0] = tab[2][0] * base;
    tab[3][1] = 0;
    tab[4][0] = tab[3][0] * base;
    tab[4][1] = lmmp_mulh_(tab[3][0], base);
    tab[5][0] = tab[3][0] * tab[2][0];
    tab[5][1] = lmmp_mulh_(tab[3][0], tab[2][0]);
    tab[6][0] = tab[3][0] * tab[3][0];
    tab[6][1] = lmmp_mulh_(tab[3][0], tab[3][0]);
    tab[7][0] = tab[6][0] * base;
    tab[7][1] = lmmp_mulh_(tab[6][0], base);
    tab[7][1] += tab[6][1] * base;

    mp_byte_t p[22];
    cal_p(0, 3);
    cal_p(5, 3);
    cal_p(10, 3);
    cal_p(15, 3);
    p[20] = exp & 0x7;
    exp >>= 3;
    p[21] = exp & 0x7;

    TEMP_DECL;
    mp_size_t dsn = 1;
    bool rsq = true; /* is true mean now sq is result */
    mp_ptr sq = TALLOC_TYPE(rn, mp_limb_t);
    sq[0] = 1;
    mp_size_t i = 21;
    while (p[i--] == 0);
    for (++i; i != (mp_size_t)-1; i--) {
        mp_srcptr tap = tab[p[i]];
        mp_size_t tan = (tap[1] != 0) ? 2 : 1;
        if (rsq) {
            if (tan >= dsn)
                lmmp_mul_basecase_(dst, tap, tan, sq, dsn);
            else
                lmmp_mul_basecase_(dst, sq, dsn, tap, tan);
            dsn += tan;
            dsn -= (dst[dsn - 1] == 0) ? 1 : 0;
            rsq = false;
        } else {
            lmmp_mul_basecase_(sq, dst, dsn, tap, tan);
            dsn += tan;
            dsn -= (sq[dsn - 1] == 0) ? 1 : 0;
            rsq = true;
        }

        if (i > 0) {
            if (rsq) {
                lmmp_sqr_(dst, sq, dsn);
                dsn <<= 1;
                dsn -= (dst[dsn - 1] == 0) ? 1 : 0;
                lmmp_sqr_(sq, dst, dsn);
                dsn <<= 1;
                dsn -= (sq[dsn - 1] == 0) ? 1 : 0;
                lmmp_sqr_(dst, sq, dsn);
                dsn <<= 1;
                dsn -= (dst[dsn - 1] == 0) ? 1 : 0;
                rsq = false;
            } else {
                lmmp_sqr_(sq, dst, dsn);
                dsn <<= 1;
                dsn -= (sq[dsn - 1] == 0) ? 1 : 0;
                lmmp_sqr_(dst, sq, dsn);
                dsn <<= 1;
                dsn -= (dst[dsn - 1] == 0) ? 1 : 0;
                lmmp_sqr_(sq, dst, dsn);
                dsn <<= 1;
                dsn -= (sq[dsn - 1] == 0) ? 1 : 0;
                rsq = true;
            }
        }
    }
    if (rsq)
        lmmp_copy(dst, sq, dsn);
    TEMP_FREE;
    return dsn;
}

mp_size_t lmmp_8_pow_1_(mp_ptr dst, mp_size_t rn, mp_limb_t base, mp_limb_t exp) {
    mp_limb_t tab[4][2];
    tab[0][0] = 1;
    tab[0][1] = 0;
    tab[1][0] = base;
    tab[1][1] = 0;
    tab[2][0] = base * base;
    tab[2][1] = 0;
    tab[3][0] = tab[2][0] * base;
    tab[3][1] = lmmp_mulh_(tab[2][0], base);
    tab[3][1] += tab[2][1] * base;

    /** 
     * 24*2 bits form exp is enough 
     * if you want to use more bits, you can chage the size of p[]
     */
    mp_byte_t p[24];
    cal_p(0, 2);
    cal_p(5, 2);
    cal_p(10, 2);
    cal_p(15, 2);
    p[20] = exp & 0x3;
    exp >>= 2;
    p[21] = exp & 0x3;
    exp >>= 2;
    p[22] = exp & 0x3;
    exp >>= 2;
    p[23] = exp & 0x3;

    TEMP_DECL;
    mp_size_t dsn = 1;
    bool rsq = true; /* is true mean now sq is result */
    mp_ptr sq = TALLOC_TYPE(rn, mp_limb_t);
    sq[0] = 1;
    mp_size_t i = 23;
    while (p[i--] == 0);
    for (++i; i != (mp_size_t)-1; i--) {
        mp_srcptr tap = tab[p[i]];
        mp_size_t tan = (tap[1] != 0) ? 2 : 1;
        if (rsq) {
            if (tan >= dsn)
                lmmp_mul_basecase_(dst, tap, tan, sq, dsn);
            else
                lmmp_mul_basecase_(dst, sq, dsn, tap, tan);
            dsn += tan;
            dsn -= (dst[dsn - 1] == 0) ? 1 : 0;
            rsq = false;
        } else {
            lmmp_mul_basecase_(sq, dst, dsn, tap, tan);
            dsn += tan;
            dsn -= (sq[dsn - 1] == 0) ? 1 : 0;
            rsq = true;
        }

        if (i > 0) {
            if (rsq) {
                lmmp_sqr_(dst, sq, dsn);
                dsn <<= 1;
                dsn -= (dst[dsn - 1] == 0) ? 1 : 0;
                lmmp_sqr_(sq, dst, dsn);
                dsn <<= 1;
                dsn -= (sq[dsn - 1] == 0) ? 1 : 0;
            } else {
                lmmp_sqr_(sq, dst, dsn);
                dsn <<= 1;
                dsn -= (sq[dsn - 1] == 0) ? 1 : 0;
                lmmp_sqr_(dst, sq, dsn);
                dsn <<= 1;
                dsn -= (dst[dsn - 1] == 0) ? 1 : 0;
            }
        }
    }
    if (rsq)
        lmmp_copy(dst, sq, dsn);
    TEMP_FREE;
    return dsn;
}

mp_size_t lmmp_16_pow_1_(mp_ptr dst, mp_size_t rn, mp_limb_t base, mp_limb_t exp) {
    mp_limb_t b2[2];
    b2[0] = base * base;
    b2[1] = lmmp_mulh_(base, base);
    mp_limb_t b3[3];
    b3[2] = lmmp_mul_1_(b3, b2, 2, base);
    mp_size_t b3n = b3[2] != 0 ? 3 : 2;

    /**
     * 20*2 bits form exp is enough
     * if you want to use more bits, you can chage the size of p[]
     */
    mp_byte_t p[20];
    cal_p(0, 2);
    cal_p(5, 2);
    cal_p(10, 2);
    p[15] = exp & 0x3;
    exp >>= 2;
    p[16] = exp & 0x3;
    exp >>= 2;
    p[17] = exp & 0x3;
    exp >>= 2;
    p[18] = exp & 0x3;
    exp >>= 2;
    p[19] = exp & 0x3;

    TEMP_DECL;
    mp_size_t dsn = 1;
    bool rsq = true; /* is true mean now sq is result */
    mp_ptr sq = TALLOC_TYPE(rn, mp_limb_t);
    sq[0] = 1;
    mp_size_t i = 19;
    while (p[i--] == 0);
    for (++i; i != (mp_size_t)-1; i--) {
        if (p[i] == 0) {
            ;
        } else if (p[i] == 1) {
            if (rsq) {
                sq[dsn] = lmmp_mul_1_(sq, sq, dsn, base);
                ++dsn;
                dsn -= (sq[dsn - 1] == 0) ? 1 : 0;
            } else {
                dst[dsn] = lmmp_mul_1_(dst, dst, dsn, base);
                ++dsn;
                dsn -= (dst[dsn - 1] == 0) ? 1 : 0;
            }
        } else if (p[i] == 2) {
            if (rsq) {
                if (2 >= dsn)
                    lmmp_mul_basecase_(dst, b2, 2, sq, dsn);
                else
                    lmmp_mul_basecase_(dst, sq, dsn, b2, 2);
                dsn += 2;
                dsn -= (dst[dsn - 1] == 0) ? 1 : 0;
                rsq = false;
            } else {
                lmmp_mul_basecase_(sq, dst, dsn, b2, 2);
                dsn += 2;
                dsn -= (sq[dsn - 1] == 0) ? 1 : 0;
                rsq = true;
            }
        } else {
            if (rsq) {
                if (b3n >= dsn)
                    lmmp_mul_basecase_(dst, b3, b3n, sq, dsn);
                else
                    lmmp_mul_basecase_(dst, sq, dsn, b3, b3n);
                dsn += b3n;
                dsn -= (dst[dsn - 1] == 0) ? 1 : 0;
                rsq = false;
            } else {
                lmmp_mul_basecase_(sq, dst, dsn, b3, b3n);
                dsn += b3n;
                dsn -= (sq[dsn - 1] == 0) ? 1 : 0;
                rsq = true;
            }
        }
        
        if (i > 0) {
            if (rsq) {
                lmmp_sqr_(dst, sq, dsn);
                dsn <<= 1;
                dsn -= (dst[dsn - 1] == 0) ? 1 : 0;
                lmmp_sqr_(sq, dst, dsn);
                dsn <<= 1;
                dsn -= (sq[dsn - 1] == 0) ? 1 : 0;
            } else {
                lmmp_sqr_(sq, dst, dsn);
                dsn <<= 1;
                dsn -= (sq[dsn - 1] == 0) ? 1 : 0;
                lmmp_sqr_(dst, sq, dsn);
                dsn <<= 1;
                dsn -= (dst[dsn - 1] == 0) ? 1 : 0;
            }
        }
    }
    if (rsq)
        lmmp_copy(dst, sq, dsn);
    TEMP_FREE;
    return dsn;
}

mp_size_t lmmp_pow_1_(mp_ptr dst, mp_limb_t base, mp_limb_t exp) {
    mp_size_t rn = ceil((double)exp * log2(base));
    rn = (rn + LIMB_BITS - 1) / LIMB_BITS + 1; /* more one limb for carry */
    if (base <= (mp_limb_t)0xf) {
        return lmmp_1_pow_1_(dst, rn, base, exp);
    } else if (base <= (mp_limb_t)0xff) {
        return lmmp_2_pow_1_(dst, rn, base, exp);
    } else if (base <= (mp_limb_t)0xffff) {
        return lmmp_4_pow_1_(dst, rn, base, exp);
    } else if (base <= (mp_limb_t)0xffffffff) {
        /** in fact, exp need to be less than 2^48, but we don't check it here.  *
         *  because such computation requires too much memory space and is more  *
         *  likely to encounter errors due to insufficient memory               **/
        return lmmp_8_pow_1_(dst, rn, base, exp);
    } else {
        /** in fact, exp need to be less than 2^40, but we don't check it here.  *
         *  because such computation requires too much memory space and is more  *
         *  likely to encounter errors due to insufficient memory               **/
        return lmmp_16_pow_1_(dst, rn, base, exp);
    }
    return 0;
}
