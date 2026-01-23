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

mp_size_t lmmp_1_pow_1_(mp_ptr dst, mp_size_t rn, ulong base, ulong exp) {
    lmmp_debug_assert(base <= 0xf && base >= 2);
    
    TEMP_DECL;
    mp_limb_t tab[16];
    tab[0] = 1;
    tab[1] = base;
    cal_tab(2);
    cal_tab(9);

    mp_size_t sqn = (rn + 2) >> 1;
    mp_size_t dsn = 1;
    mp_ptr sq = TALLOC_TYPE(sqn, mp_limb_t);
    dst[0] = 1;
    int i = 15;
    while( (exp & (0xfull << ((i--) * 4))) == 0);
    for (++i; i >= 0; --i) {
        mp_size_t p = (exp & (0xfull << (i * 4))) >> (i * 4);
        dst[dsn] = lmmp_mul_1_(dst, dst, dsn, tab[p]);
        ++dsn;
        dsn -= (dst[dsn - 1] == 0) ? 1 : 0;
        if (i > 0) {
            lmmp_sqr_(sq, dst, dsn);
            dsn <<= 1;
            dsn -= (sq[dsn - 1] == 0) ? 1 : 0;

            lmmp_sqr_(dst, sq, dsn);
            dsn <<= 1;
            dsn -= (dst[dsn - 1] == 0) ? 1 : 0;
            
            lmmp_sqr_(sq, dst, dsn);
            dsn <<= 1;
            lmmp_debug_assert(dsn <= sqn);
            dsn -= (sq[dsn - 1] == 0) ? 1 : 0;
            
            lmmp_sqr_(dst, sq, dsn);
            dsn <<= 1;
            dsn -= (dst[dsn - 1] == 0) ? 1 : 0;
        }
    }
    TEMP_FREE;
    return dsn;
}

mp_size_t lmmp_2_pow_1_(mp_ptr dst, mp_size_t rn, ulong base, ulong exp) {
    lmmp_debug_assert(base >= 2);
    lmmp_debug_assert(base <= 0xff);
    TEMP_DECL;
    mp_limb_t tab[8];
    tab[0] = 1;
    cal_tab(1);

    mp_size_t dsn = 1;
    mp_ptr sq = TALLOC_TYPE(rn, mp_limb_t);
    sq[0] = 1;
    int i = 21;
    while ((exp & (0x7ull << ((i--) * 3))) == 0);
    for (++i; i >= 0; --i) {
        mp_size_t p = (exp & (0x7ull << (i * 3))) >> (i * 3);
            dst[dsn] = lmmp_mul_1_(dst, sq, dsn, tab[p]);
            ++dsn;
            dsn -= (dst[dsn - 1] == 0) ? 1 : 0;
        if (i > 0) {
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
    TEMP_FREE;
    return dsn;
}

mp_size_t lmmp_4_pow_1_(mp_ptr dst, mp_size_t rn, ulong base, ulong exp) {
    lmmp_debug_assert(base >= 2);
    lmmp_debug_assert(base <= 0xffff);
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

    mp_size_t dsn = 1;
    mp_ptr sq = TALLOC_TYPE(rn, mp_limb_t);
    dst[0] = 1;
    int i = 21;
    while ((exp & (0x7ull << ((i--) * 3))) == 0);
    for (++i; i >= 0; --i) {
        mp_size_t p = (exp & (0x7ull << (i * 3))) >> (i * 3);
        mp_srcptr tap = tab[p];
        mp_size_t tan = (tap[1] != 0) ? 2 : 1;
        if (dsn >= tan)
            lmmp_mul_basecase_(sq, dst, dsn, tap, tan);
        else 
            lmmp_mul_basecase_(sq, tap, tan, dst, dsn);
        dsn += tan;
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
        }
    }
    lmmp_copy(dst, sq, dsn);
    TEMP_FREE;
    return dsn;
}

mp_size_t lmmp_8_pow_1_(mp_ptr dst, mp_size_t rn, ulong base, ulong exp) {
    lmmp_debug_assert(base >= 2);
    lmmp_debug_assert(base <= 0xffffffff);
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

    mp_size_t dsn = 1;
    bool rsq = true; /* is true mean now sq is result */
    mp_ptr sq = TALLOC_TYPE(rn, mp_limb_t);
    sq[0] = 1;
    int i = 31;
    while ((exp & (0x3ull << ((i--) * 2))) == 0);
    for (++i; i >= 0; --i) {
        mp_size_t p = (exp & (0x3ull << (i * 2))) >> (i * 2);
        mp_srcptr tap = tab[p];
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

mp_size_t lmmp_16_pow_1_(mp_ptr dst, mp_size_t rn, ulong base, ulong exp) {
    lmmp_debug_assert(base > 0xffffffff);
    TEMP_DECL;

    mp_limb_t b2[2];
    lmmp_mullh_(base, base, b2);
    mp_limb_t b3[3];
    b3[2] = lmmp_mul_1_(b3, b2, 2, base);
    mp_size_t b3n = b3[2] != 0 ? 3 : 2;

    mp_size_t dsn = 1;
    bool rsq = true; /* is true mean now sq is result */
    mp_ptr sq = TALLOC_TYPE(rn, mp_limb_t);
    sq[0] = 1;
    int i = 31;
    while ((exp & (0x3ull << ((i--) * 2))) == 0);
    for (++i; i >= 0; --i) {
        mp_size_t p = (exp & (0x3ull << (i * 2))) >> (i * 2);
        if (p == 0) {
            ;
        } else if (p == 1) {
            if (rsq) {
                sq[dsn] = lmmp_mul_1_(sq, sq, dsn, base);
                ++dsn;
                dsn -= (sq[dsn - 1] == 0) ? 1 : 0;
            } else {
                dst[dsn] = lmmp_mul_1_(dst, dst, dsn, base);
                ++dsn;
                dsn -= (dst[dsn - 1] == 0) ? 1 : 0;
            }
        } else if (p == 2) {
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
