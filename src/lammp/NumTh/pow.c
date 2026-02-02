#include "../../../include/lammp/numth.h"

mp_size_t lmmp_pow_(mp_ptr dst, mp_size_t rn, mp_srcptr base, mp_size_t n, ulong exp) {
    lmmp_debug_assert(n > 0);
    lmmp_debug_assert(exp > 0);
    lmmp_debug_assert(base[n - 1] != 0);
    if (exp == 1) {
        lmmp_copy(dst, base, n);
        return n;
    } else if (exp == 2) {
        lmmp_sqr_(dst, base, n);
        rn = n << 1;
        rn -= (dst[rn - 1] == 0);
        return rn;
    } else {
        if (n == 1) {
            if (exp <= POW_1_EXP_THRESHOLD) {
                dst[0] = base[0];
                rn = 1;
                for (mp_size_t i = 1; i < exp; ++i) {
                    dst[rn] = lmmp_mul_1_(dst, dst, rn, base[0]);
                    ++rn;
                    rn -= (dst[rn - 1] == 0);
                }
                return rn;
            } else {
                return lmmp_pow_1_(dst, rn, base[0], exp);
            }
        }
        if (exp > POW_WIN2_EXP_THRESHOLD && n > POW_WIN2_N_THRESHOLD) {
            if ((exp % 4 == 3) || (2 * lmmp_limb_popcnt_(exp) >= (lmmp_limb_bits_(exp)))) {
                return lmmp_pow_win2_(dst, rn, base, n, exp);
            }
        }
        if (exp & 1) {
            return lmmp_pow_basecase_(dst, rn, base, n, exp);
        }

        mp_size_t tz = lmmp_tailing_zeros_(exp);
        TEMP_DECL;
        mp_ptr sq = TALLOC_TYPE(rn, mp_limb_t);
        exp >>= tz;
        if (exp == 1) {
            lmmp_copy(dst, base, n);
            rn = n;
        } else {
            mp_size_t rn1 = lmmp_pow_size_(base, n, exp);
            rn = lmmp_pow_basecase_(dst, rn1, base, n, exp);
        }
        mp_size_t i = 2;
        for ( ; i <= tz; i += 2) {
            lmmp_sqr_(sq, dst, rn);
            rn <<= 1;
            rn -= (sq[rn - 1] == 0);
            lmmp_sqr_(dst, sq, rn);
            rn <<= 1;
            rn -= (dst[rn - 1] == 0);
        }
        if (tz & 1) {
            lmmp_sqr_(sq, dst, rn);
            rn <<= 1;
            rn -= (sq[rn - 1] == 0);
            lmmp_copy(dst, sq, rn);
        }
        TEMP_FREE;
        return rn;
    }
}
