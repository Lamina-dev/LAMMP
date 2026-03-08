#include "../../../include/lammp/impl/heap.h"
#include "../../../include/lammp/impl/prime_table.h"
#include "../../../include/lammp/impl/factors_mul.h"

#define LOG2_ 0.693147180559945

mp_size_t lmmp_multinomial_size_(const uintp r, uint m, ulong* n) {
    *n = 0;
    uint i = 0;
    for (; i < m; ++i) *n += r[i];

    double logr = lgamma(*n + 1.0);
    
    for (i = 0; i < m; ++i) 
        logr -= lgamma(r[i] + 1.0);

    logr /= LOG2_;

    mp_size_t rn = ceil(logr / LIMB_BITS) + 2;
    return rn;
}

mp_size_t lmmp_multinomial_(mp_ptr dst, mp_size_t rn, uint n, const uintp r, uint m) {
    lmmp_param_assert(m > 0 && n > 0);
    if (n <= 20) {
        lmmp_nPr_short_(dst, rn, n, n);
        mp_limb_t t = 0;
        for (uint i = 0; i < m; ++i) {
            lmmp_nPr_short_(&t, 1, r[i], r[i]);
            dst[0] /= t;
        }
        return 1;
    }

    TEMP_B_DECL;
    uint nfactors_max = lmmp_prime_cnt_table_(n) - 1;
    uint nfactors = 0;
    factors fac = BALLOC_TYPE(nfactors_max, factor);
    /*
        对于2这个因子，我们单独处理，因为可以通过移位来计算。
     */
    for (uint i = 0; i < nfactors_max; ++i) {
        uint prime = lmmp_nth_prime_table_(i + 1);
        fac[i].j = 0;
        uint pn = n;
        uint e = 0;
        while (pn > 0) {
            pn /= prime;
            e += pn;
        }
        for (uint j = 0; j < m; ++j) {
            pn = r[j];
            while (pn > 0) {
                pn /= prime;
                e -= pn;
            }
        }
        if (e > 0) {
            fac[nfactors].f = prime;
            fac[nfactors++].j = e;
        }
    }

    mp_size_t shl = n - lmmp_limb_popcnt_(n);
    for (uint j = 0; j < m; ++j) {
        shl += lmmp_limb_popcnt_(r[j]);
        shl -= r[j];
    }
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
