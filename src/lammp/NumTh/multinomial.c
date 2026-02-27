#include "../../../include/lammp/impl/heap.h"
#include "../../../include/lammp/impl/prime_table.h"

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

mp_size_t lmmp_multinomial_short_(mp_ptr dst, mp_size_t rn, uint n, const uintp r, uint m) {
    lmmp_param_assert(n <= 0xffff);
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
    pri_short primes;
    lmmp_prime_short_init_(&primes, n);
    num_heap heap;
    lmmp_num_heap_init_(&heap, primes.prin);

    mp_size_t mpn = 1;
    mp_ptr mp = ALLOC_TYPE(PERMUTATION_MUL_MAX_THRESHOLD, mp_limb_t);
    mp[0] = 1;
    /* 跳过质数 2 */
    for (ulong i = 1; i < primes.prin; ++i) {
        ulong pn = n;
        ulong e = 0;
        while (pn > 0) {
            pn /= primes.pri[i];
            e += pn;
        }
        for (uint j = 0; j < m; ++j) {
            pn = r[j];
            while (pn > 0) {
                pn /= primes.pri[i];
                e -= pn;
            }
        }

        if (e == 0) {
            continue;
        } else if (e == 1) {
            mp[mpn] = lmmp_mul_1_(mp, mp, mpn, primes.pri[i]);
            ++mpn;
            mpn -= mp[mpn - 1] == 0 ? 1 : 0;
        } else if (e == 2) {
            mp[mpn] = lmmp_mul_1_(mp, mp, mpn, (mp_limb_t)(primes.pri[i]) * primes.pri[i]);
            ++mpn;
            mpn -= mp[mpn - 1] == 0 ? 1 : 0;
        } else if (e == 3) {
            mp[mpn] = lmmp_mul_1_(mp, mp, mpn, (mp_limb_t)(primes.pri[i]) * primes.pri[i] * primes.pri[i]);
            ++mpn;
            mpn -= mp[mpn - 1] == 0 ? 1 : 0;
        } else if (e == 4) {
            mp_limb_t pri2 = (mp_limb_t)primes.pri[i] * primes.pri[i];
            mp[mpn] = lmmp_mul_1_(mp, mp, mpn, pri2 * pri2);
            ++mpn;
            mpn -= mp[mpn - 1] == 0 ? 1 : 0;
        } else if (e >= PERMUTATION_PRIME_POW_THRESHOLD) {
            mp_size_t pon = lmmp_pow_1_size_(primes.pri[i], e);
            mp_ptr po = ALLOC_TYPE(pon, mp_limb_t);
            pon = lmmp_pow_1_(po, pon, primes.pri[i], e);
            lmmp_num_heap_push_(&heap, po, pon);
            continue;
        } else {
            mp_size_t pon = lmmp_pow_1_size_(primes.pri[i], e);
            mp_ptr po = ALLOC_TYPE(pon, mp_limb_t);
            pon = 1;
            mp_limb_t p3 = (mp_limb_t)primes.pri[i] * primes.pri[i] * primes.pri[i];
            po[0] = p3;
            uint j = 3;
            for (; j < e - 2; j += 3) {
                po[pon] = lmmp_mul_1_(po, po, pon, p3);
                ++pon;
                pon -= po[pon - 1] == 0 ? 1 : 0;
            }
            for (; j < e; ++j) {
                po[pon] = lmmp_mul_1_(po, po, pon, primes.pri[i]);
                ++pon;
                pon -= po[pon - 1] == 0 ? 1 : 0;
            }
            lmmp_num_heap_push_(&heap, po, pon);
            continue;
        }
        if (mpn == PERMUTATION_MUL_MAX_THRESHOLD) {
            lmmp_num_heap_push_(&heap, mp, mpn);
            mp = ALLOC_TYPE(PERMUTATION_MUL_MAX_THRESHOLD, mp_limb_t);
            mpn = 1;
            mp[0] = 1;
        }
    }
    if (!(mpn == 1 && mp[0] == 1))
        lmmp_num_heap_push_(&heap, mp, mpn);
    else
        lmmp_free(mp);

    lmmp_prime_short_free_(&primes);
    lmmp_debug_assert(heap.size != 0);
    mp = lmmp_num_heap_mul_(&heap, &mpn);
    lmmp_num_heap_free_(&heap);
    
    /* 乘以 2 的幂次方 */
    rn = n - lmmp_limb_popcnt_(n);
    for (uint j = 0; j < m; ++j) {
        rn += lmmp_limb_popcnt_(r[j]);
        rn -= r[j];
    }

    mp_size_t sh_w = rn / LIMB_BITS;
    rn %= LIMB_BITS;
    lmmp_zero(dst, sh_w);
    dst[sh_w + mpn] = lmmp_shl_(dst + sh_w, mp, mpn, rn);
    sh_w += mpn + 1;
    sh_w -= dst[sh_w - 1] == 0 ? 1 : 0;

    lmmp_free(mp);
    return sh_w;
}

mp_size_t lmmp_multinomial_int_(mp_ptr dst, mp_size_t rn, uint n, const uintp r, uint m) {
    lmmp_param_assert(n > 0xffff);
    lmmp_param_assert(m > 1);
    pri_int primes;
    lmmp_prime_int_init_(&primes, n);
    num_heap heap;
    lmmp_num_heap_init_(&heap, primes.prin);

    mp_size_t mpn = 1;
    mp_ptr mp = ALLOC_TYPE(PERMUTATION_MUL_MAX_THRESHOLD, mp_limb_t);
    mp[0] = 1;
    /* 跳过质数 2 */
    for (ulong i = 1; i < primes.prin; ++i) {
        ulong pn = n;
        ulong e = 0;
        while (pn > 0) {
            pn /= primes.pri[i];
            e += pn;
        }
        for (uint j = 0; j < m; ++j) {
            pn = r[j];
            while (pn > 0) {
                pn /= primes.pri[i];
                e -= pn;
            }
        }

        if (e == 0) {
            continue;
        } else if (e == 1) {
            mp[mpn] = lmmp_mul_1_(mp, mp, mpn, primes.pri[i]);
            ++mpn;
            mpn -= mp[mpn - 1] == 0 ? 1 : 0;
        } else if (e == 2) {
            mp[mpn] = lmmp_mul_1_(mp, mp, mpn, (mp_limb_t)(primes.pri[i]) * primes.pri[i]);
            ++mpn;
            mpn -= mp[mpn - 1] == 0 ? 1 : 0;
        } else if (e >= PERMUTATION_PRIME_POW_THRESHOLD) {
            mp_size_t pon = lmmp_pow_1_size_(primes.pri[i], e);
            mp_ptr po = ALLOC_TYPE(pon, mp_limb_t);
            pon = lmmp_pow_1_(po, pon, primes.pri[i], e);
            lmmp_num_heap_push_(&heap, po, pon);
            continue;
        } else {
            mp_size_t pon = lmmp_pow_1_size_(primes.pri[i], e);
            mp_ptr po = ALLOC_TYPE(pon, mp_limb_t);
            pon = 1;
            mp_limb_t pri2 = (mp_limb_t)primes.pri[i] * primes.pri[i];
            po[0] = pri2;
            for (uint j = 2; j < e - 1; j += 2) {
                po[pon] = lmmp_mul_1_(po, po, pon, pri2);
                ++pon;
                pon -= po[pon - 1] == 0 ? 1 : 0;
            }
            if (e % 2 == 1) {
                po[pon] = lmmp_mul_1_(po, po, pon, primes.pri[i]);
                ++pon;
                pon -= po[pon - 1] == 0 ? 1 : 0;
            }
            lmmp_num_heap_push_(&heap, po, pon);
            continue;
        }

        if (mpn == PERMUTATION_MUL_MAX_THRESHOLD) {
            lmmp_num_heap_push_(&heap, mp, mpn);
            mp = ALLOC_TYPE(PERMUTATION_MUL_MAX_THRESHOLD, mp_limb_t);
            mpn = 1;
            mp[0] = 1;
        }
    }
    if (!(mpn == 1 && mp[0] == 1))
        lmmp_num_heap_push_(&heap, mp, mpn);
    else
        lmmp_free(mp);

    lmmp_prime_int_free_(&primes);

    mp = lmmp_num_heap_mul_(&heap, &mpn);
    lmmp_num_heap_free_(&heap);

    /* 乘以 2 的幂次方 */
    rn = n - lmmp_limb_popcnt_(n);
    for (uint j = 0; j < m; ++j) {
        rn += lmmp_limb_popcnt_(r[j]);
        rn -= r[j];
    }

    mp_size_t sh_w = rn / LIMB_BITS;
    rn %= LIMB_BITS;
    lmmp_zero(dst, sh_w);
    dst[sh_w + mpn] = lmmp_shl_(dst + sh_w, mp, mpn, rn);
    sh_w += mpn + 1;
    sh_w -= dst[sh_w - 1] == 0 ? 1 : 0;

    lmmp_free(mp);
    return sh_w;
}