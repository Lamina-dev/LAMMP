#include "../../../include/lammp/impl/factors_mul.h"
#include "../../../include/lammp/impl/heap.h"
#include "../../../include/lammp/impl/prime_table.h"
#include "../../../include/lammp/numth.h"

mp_size_t lmmp_nCr_short_(mp_ptr restrict dst, mp_size_t rn, uint n, uint r) {
    lmmp_param_assert(n <= 0xffff);
    lmmp_param_assert(r <= n / 2);
    if (r <= 20) {
        rn = lmmp_nPr_short_(dst, rn, n, r);
        mp_limb_t t = 0;
        lmmp_nPr_short_(&t, 1, r, r);
        lmmp_div_1_(dst, dst, rn, t);
        rn -= dst[rn - 1] == 0 ? 1 : 0;
        return rn;
    } else if (rn < BINOMIAL_RN_BASECASE_THRESHOLD) {
        if (r <= 4 || n > 0xfff) {
            dst[0] = 1;
            rn = 1;
            for (ulong i = 1; i <= r; ++i) {
                ulong t = n - i + 1;
                dst[rn] = lmmp_mul_1_(dst, dst, rn, t);
                ++rn;
                rn -= dst[rn - 1] == 0 ? 1 : 0;
                lmmp_div_1_(dst, dst, rn, i);
                rn -= dst[rn - 1] == 0 ? 1 : 0;
            }
            return rn;
        } else {
            dst[0] = 1;
            rn = 1;
            ulong i = 1;
            ulong t = 1, d = 1;
            for (; i <= (ulong)r - 4; i += 4) {
                d = i * (i + 1) * (i + 2) * (i + 3);
                d /= 12;
                t = (n - i + 1) * (n - i) * (n - i - 1) * (n - i - 2);
                t /= 12;
                dst[rn] = lmmp_mul_1_(dst, dst, rn, t);
                ++rn;
                rn -= dst[rn - 1] == 0 ? 1 : 0;
                lmmp_div_1_(dst, dst, rn, d);
                rn -= dst[rn - 1] == 0 ? 1 : 0;
            }
            for (; i <= r; ++i) {
                t = n - i + 1;
                dst[rn] = lmmp_mul_1_(dst, dst, rn, t);
                ++rn;
                rn -= dst[rn - 1] == 0 ? 1 : 0;
                lmmp_div_1_(dst, dst, rn, i);
                rn -= dst[rn - 1] == 0 ? 1 : 0;
            }
            return rn;
        }
    } else {
        TEMP_DECL;
        uint nfactors = lmmp_prime_size_(n);
        factors restrict fac = TALLOC_TYPE(nfactors, factor);
        uint nr = n - r;
        /*
            对于2这个因子，我们单独处理，因为可以通过移位来计算。
         */
        nfactors = 0;
        for (uint i = 3; i <= n; ++i) {
            if (!lmmp_is_prime_table_(i))
                continue;
            uint pn = n;
            uint e = 0;
            while (pn > 0) {
                pn /= i;
                e += pn;
            }
            pn = r;
            while (pn > 0) {
                pn /= i;
                e -= pn;
            }
            pn = nr;
            while (pn > 0) {
                pn /= i;
                e -= pn;
            }
            if (e > 0) {
                fac[nfactors].f = i;
                fac[nfactors++].j = e;
            }
        }

        mp_size_t shl = n - lmmp_limb_popcnt_(n);
        shl -= r - lmmp_limb_popcnt_(r);
        shl -= nr - lmmp_limb_popcnt_(nr);
        mp_size_t shw = shl / LIMB_BITS;
        shl %= LIMB_BITS;

        lmmp_zero(dst, shw);
        rn = lmmp_factors_mul_(dst + shw, rn - shw, fac, nfactors, n);

        dst[shw + rn] = lmmp_shl_(dst + shw, dst + shw, rn, shl);
        rn += shw + 1;
        rn -= dst[rn - 1] == 0 ? 1 : 0;

        TEMP_FREE;
        return rn;
    }
}

mp_size_t lmmp_nCr_int_(mp_ptr restrict dst, mp_size_t rn, uint n, uint r) {
    lmmp_param_assert(r <= (n / 2));
    if (r <= 3 || n > 0xfffffff) {
        dst[0] = 1;
        rn = 1;
        for (ulong i = 1; i <= r; ++i) {
            ulong t = n - i + 1;
            dst[rn] = lmmp_mul_1_(dst, dst, rn, t);
            ++rn;
            rn -= dst[rn - 1] == 0 ? 1 : 0;
            lmmp_div_1_(dst, dst, rn, i);
            rn -= dst[rn - 1] == 0 ? 1 : 0;
        }
        return rn;
    } else if (rn < BINOMIAL_RN_BASECASE_THRESHOLD) {
        dst[0] = 1;
        rn = 1;
        ulong i = 1;
        ulong t = 1, d = 1;
        for (; i <= (ulong)r - 2; i += 2) {
            d = i * (i + 1);
            d >>= 1;
            t = (n - i + 1) * (n - i);
            t >>= 1;
            dst[rn] = lmmp_mul_1_(dst, dst, rn, t);
            ++rn;
            rn -= dst[rn - 1] == 0 ? 1 : 0;
            lmmp_div_1_(dst, dst, rn, d);
            rn -= dst[rn - 1] == 0 ? 1 : 0;
        }
        for (; i <= r; ++i) {
            t = n - i + 1;
            dst[rn] = lmmp_mul_1_(dst, dst, rn, t);
            ++rn;
            rn -= dst[rn - 1] == 0 ? 1 : 0;
            lmmp_div_1_(dst, dst, rn, i);
            rn -= dst[rn - 1] == 0 ? 1 : 0;
        }
        return rn;
    } else {
        lmmp_prime_int_table_init_(n);
        TEMP_B_DECL;
        uint nfactors = lmmp_prime_size_(n);
        factors restrict fac = BALLOC_TYPE(nfactors, factor);
        uint nr = n - r;
        /*
            对于2这个因子，我们单独处理，因为可以通过移位来计算。
         */
        nfactors = 0;
        for (uint i = 3; i <= n; ++i) {
            if (!lmmp_is_prime_table_(i))
                continue;
            uint pn = n;
            uint e = 0;
            while (pn > 0) {
                pn /= i;
                e += pn;
            }
            pn = r;
            while (pn > 0) {
                pn /= i;
                e -= pn;
            }
            pn = nr;
            while (pn > 0) {
                pn /= i;
                e -= pn;
            }
            if (e > 0) {
                fac[nfactors].j = e;
                fac[nfactors++].f = i;
            }
        }

        mp_size_t shl = n - lmmp_limb_popcnt_(n);
        shl -= r - lmmp_limb_popcnt_(r);
        shl -= nr - lmmp_limb_popcnt_(nr);
        mp_size_t shw = shl / LIMB_BITS;
        shl %= LIMB_BITS;

        lmmp_zero(dst, shw);
        rn = lmmp_factors_mul_(dst + shw, rn - shw, fac, nfactors, n);

        dst[shw + rn] = lmmp_shl_(dst + shw, dst + shw, rn, shl);
        rn += shw + 1;
        rn -= dst[rn - 1] == 0 ? 1 : 0;

        TEMP_B_FREE;
        return rn;
    }
}