#include "../../../include/lammp/lmmpn.h"
#include "../../../include/lammp/numth.h"

mp_size_t lmmp_pow_(mp_ptr restrict dst, mp_size_t rn, mp_srcptr restrict base, mp_size_t n, ulong exp) {
    lmmp_param_assert(n > 0);
    lmmp_param_assert(exp > 0);
    lmmp_param_assert(base[n - 1] != 0);
    if (exp == 1) {
        lmmp_copy(dst, base, n);
        return n;
    } else if (exp == 2) {
        lmmp_sqr_(dst, base, n);
        rn = n << 1;
        rn -= (dst[rn - 1] == 0);
        return rn;
    } else {
        mp_size_t base_tz = 0;
        while (*base == 0) {
            ++base_tz;
            ++base;
            --n;
        }
        base_tz *= exp;
        lmmp_zero(dst, base_tz);
        dst += base_tz;
        if (n == 1) {
            if (exp <= POW_1_EXP_THRESHOLD) {
                dst[0] = base[0];
                rn = 1;
                for (mp_size_t i = 1; i < exp; ++i) {
                    dst[rn] = lmmp_mul_1_(dst, dst, rn, base[0]);
                    ++rn;
                    rn -= (dst[rn - 1] == 0);
                }
                return rn + base_tz;
            } else {
                return lmmp_pow_1_(dst, rn, base[0], exp) + base_tz;
            }
        } else { /* n > 2 */
            if (exp > POW_WIN2_EXP_THRESHOLD && n > POW_WIN2_N_THRESHOLD) {
                if ((exp % 4 == 3) || (2 * lmmp_limb_popcnt_(exp) >= (lmmp_limb_bits_(exp)))) {
                    return lmmp_pow_win2_(dst, rn, base, n, exp) + base_tz;
                }
            }
            if (exp & 1) {
                return lmmp_pow_basecase_(dst, rn, base, n, exp) + base_tz;
            }

            int tz = lmmp_tailing_zeros_(exp);
            TEMP_DECL;
            mp_ptr restrict sq = TALLOC_TYPE((rn + 2) >> 1, mp_limb_t);
            exp >>= tz;

            if (tz & 1) {
                if (exp == 1) {
                    lmmp_copy(sq, base, n);
                    rn = n;
                } else {
                    mp_size_t rn1 = lmmp_pow_size_(base, n, exp);
                    rn = lmmp_pow_basecase_(sq, rn1, base, n, exp);
                }
                int i = 2;
                for (; i <= tz; i += 2) {
                    lmmp_sqr_(dst, sq, rn);
                    rn <<= 1;
                    rn -= (dst[rn - 1] == 0);
                    lmmp_sqr_(sq, dst, rn);
                    rn <<= 1;
                    rn -= (sq[rn - 1] == 0);
                }
                lmmp_sqr_(dst, sq, rn);
                rn <<= 1;
                rn -= (dst[rn - 1] == 0);
            } else {
                if (exp == 1) {
                    lmmp_copy(dst, base, n);
                    rn = n;
                } else {
                    mp_size_t rn1 = lmmp_pow_size_(base, n, exp);
                    rn = lmmp_pow_basecase_(dst, rn1, base, n, exp);
                }
                int i = 2;
                for (; i <= tz; i += 2) {
                    lmmp_sqr_(sq, dst, rn);
                    rn <<= 1;
                    rn -= (sq[rn - 1] == 0);
                    lmmp_sqr_(dst, sq, rn);
                    rn <<= 1;
                    rn -= (dst[rn - 1] == 0);
                }
            }

            TEMP_FREE;
            return rn + base_tz;
        }
        
    }
}
