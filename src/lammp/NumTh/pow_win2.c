#include "../../../include/lammp/numth.h"

#define mul_b(_i_)                                      \
    if (rsq) {                                          \
        if (b##_i_##n >= dsn)                           \
            lmmp_mul_(dst, b##_i_, b##_i_##n, sq, dsn); \
        else                                            \
            lmmp_mul_(dst, sq, dsn, b##_i_, b##_i_##n); \
        dsn += b##_i_##n;                               \
        dsn -= (dst[dsn - 1] == 0) ? 1 : 0;             \
        rsq = false;                                    \
    } else {                                            \
        if (b##_i_##n >= dsn)                           \
            lmmp_mul_(sq, b##_i_, b##_i_##n, dst, dsn); \
        else                                            \
            lmmp_mul_(sq, dst, dsn, b##_i_, b##_i_##n); \
        dsn += b##_i_##n;                               \
        dsn -= (sq[dsn - 1] == 0) ? 1 : 0;              \
        rsq = true;                                     \
    }

mp_size_t lmmp_pow_win2_(mp_ptr dst, mp_srcptr base, mp_size_t n, ulong exp) {
    TEMP_DECL;

#define b1 base
#define b1n n
#define new_b(_i_) mp_ptr b##_i_ = TALLOC_TYPE(b##_i_##n, mp_limb_t)

    mp_size_t b2n = n << 1, b3n = b2n + n;
    new_b(2);
    new_b(3);

    lmmp_sqr_basecase_(b2, b1, b1n);
    b2n = n << 1;
    b2n -= b2[b2n - 1] == 0 ? 1 : 0;
    lmmp_mul_basecase_(b3, b2, b2n, b1, b1n);
    b3n = b2n + n;
    b3n -= b3[b3n - 1] == 0 ? 1 : 0;

    mp_size_t dsn = lmmp_pow_size_(base, n, exp);
    bool rsq = true;
    mp_ptr sq = TALLOC_TYPE(dsn, mp_limb_t);
    dsn = 1;
    sq[0] = 1;
    mp_size_t i = 31;
    while ((exp & (0x3ull << ((i--) * 2))) == 0);
    for (++i; i != (mp_size_t)-1; --i) {
        mp_size_t p = (exp & (0x3ull << (i * 2))) >> (i * 2);
        if (p == 0) {
            ;
        } else if (p == 1) {
            mul_b(1)
        } else if (p == 2) {
            mul_b(2)
        } else {
            mul_b(3)
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

#undef b1
#undef b1n
#undef new_b
}