#include "../../../include/lammp/numth.h"

mp_size_t lmmp_pow_basecase_(mp_ptr dst, mp_srcptr base, mp_size_t n, ulong exp) {
    TEMP_DECL;
    mp_size_t rn = lmmp_pow_size_(base, n, exp);
    mp_size_t sqn = n, dsn = 1;
    bool sq = true;
    mp_ptr sq1 = TALLOC_TYPE(rn, mp_limb_t);
    mp_ptr sq2 = TALLOC_TYPE(rn, mp_limb_t);
    mp_ptr re = TALLOC_TYPE(rn, mp_limb_t);
    lmmp_zero(sq1 + n, rn - n);
    lmmp_copy(sq1, base, n);
    //lmmp_zero(sq2, rn); // not necessary
    //lmmp_zero(re, rn); // not necessary
    lmmp_zero(dst + 1, rn - 1);
    dst[0] = 1;
    /*
        while (exp) {
            if (exp & 1)
                dst = dst * base
            base = (base * base) 
            exp >>= 1;
        }
        the last step base will be overflowed, so we use this :
        while (1) {
            if (exp & 1)
                dst = dst * base
            exp >>= 1;
            if (exp == 0) break;
            base = (base * base) 
        }
     */
    while (1) {
        if (exp & 1) {
            /* sqn >= dsn */
            if (sq) {
                lmmp_mul_(re, sq1, sqn, dst, dsn);
            } else {
                lmmp_mul_(re, sq2, sqn, dst, dsn);
            }
            dsn += sqn;
            dsn -= (re[dsn - 1] == 0) ? 1 : 0;
            lmmp_copy(dst, re, dsn);
        } 

        exp >>= 1;
        if (exp == 0) break;

        if (sq) {
            lmmp_sqr_(sq2, sq1, sqn);
            sq = false;
            sqn <<= 1;
            sqn -= (sq2[sqn - 1] == 0) ? 1 : 0;
        } else {
            lmmp_sqr_(sq1, sq2, sqn);
            sq = true;
            sqn <<= 1;
            sqn -= (sq1[sqn - 1] == 0) ? 1 : 0;
        }  
    }

    TEMP_FREE;
    return dsn;
}
