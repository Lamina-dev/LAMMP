#include "../../../include/lammp/numth.h"

#define mul_b(_i_)                                  \
    if (b##_i_##n >= dsn)                           \
        lmmp_mul_(sq, b##_i_, b##_i_##n, dst, dsn); \
    else                                            \
        lmmp_mul_(sq, dst, dsn, b##_i_, b##_i_##n); \
    dsn += b##_i_##n;                               \
    dsn -= (sq[dsn - 1] == 0) ? 1 : 0

mp_size_t lmmp_pow_win3_(mp_ptr dst, mp_srcptr base, mp_size_t n, ulong exp) {
    TEMP_DECL;

#define b1 base
#define b1n n
#define new_b(_i_) mp_ptr b##_i_ = TALLOC_TYPE(b##_i_##n, mp_limb_t)

    mp_size_t b2n = n << 1, b3n = b2n + n, b4n = b2n << 1;
    mp_size_t b5n = b2n + b3n, b6n = b3n << 1, b7n = b6n + b1n;
    new_b(2);
    new_b(3);
    new_b(4);
    new_b(5);
    new_b(6);
    new_b(7);
    
    lmmp_sqr_basecase_(b2, base, n);
    b2n = n << 1;
    b2n -= b2[b2n - 1] == 0 ? 1 : 0;
    lmmp_mul_basecase_(b3, b2, b2n, base, n);
    b3n = b2n + n;
    b3n -= b3[b3n - 1] == 0 ? 1 : 0;
    lmmp_sqr_basecase_(b4, b2, b2n);
    b4n = b2n << 1;
    b4n -= b4[b4n - 1] == 0 ? 1 : 0;
    lmmp_mul_basecase_(b5, b3, b3n, b2, b2n);
    b5n = b3n + b2n;
    b5n -= b5[b5n - 1] == 0 ? 1 : 0;
    lmmp_sqr_basecase_(b6, b3, b3n);
    b6n = b3n << 1;
    b6n -= b6[b6n - 1] == 0 ? 1 : 0;
    lmmp_mul_basecase_(b7, b4, b4n, b3, b3n);
    b7n = b4n + b3n;
    b7n -= b7[b7n - 1] == 0 ? 1 : 0;

    mp_size_t dsn = lmmp_pow_size_(base, n, exp);
    mp_ptr sq = TALLOC_TYPE(dsn, mp_limb_t);
    dsn = 1;
    dst[0] = 1;
    mp_size_t i = 21;
    while ((exp & (0x7ull << ((i--) * 3))) == 0);
    for (++i; i != (mp_size_t)-1; --i) {
        mp_size_t p = (exp & (0x7ull << (i * 3))) >> (i * 3);
        if (p == 0) {
            lmmp_copy(sq, dst, dsn);
        } else if (p == 1) {
            mul_b(1);
        } else if (p == 2) {
            mul_b(2);
        } else if (p == 3) {
            mul_b(3);
        } else if (p == 4) {
            mul_b(4);
        } else if (p == 5) {
            mul_b(5);
        } else if (p == 6) {
            mul_b(6);
        } else {
            mul_b(7);
        }

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

#undef b1
#undef b1n 
#undef new_b
}