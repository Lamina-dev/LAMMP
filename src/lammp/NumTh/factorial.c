#include "../../../include/lammp/numth.h"

mp_size_t lmmp_factorial_short_(mp_ptr dst, mp_size_t rn, ushort n) {
    pri_short primes;
    lmmp_prime_short_init_(&primes, n);
    num_heap heap;
    lmmp_num_heap_init_(&heap, primes.prin);

    /* 跳过质数 2 */
    for (uint i = 1; i < primes.prin; ++i) {
        uint pn = n;
        uint e = 0;
        while (pn > 0) {
            pn /= primes.pri[i];
            e += pn;
        }
        if (e == 1) {
            mp_ptr pe = ALLOC_TYPE(1, mp_limb_t);
            pe[0] = primes.pri[i];
            lmmp_num_heap_push_(&heap, pe, 1);
        } else if (e == 2) {
            mp_ptr pe = ALLOC_TYPE(1, mp_limb_t);
            pe[0] = (mp_limb_t)primes.pri[i] * primes.pri[i];
            lmmp_num_heap_push_(&heap, pe, 1);
        } else if (e == 3) {
            mp_ptr pe = ALLOC_TYPE(1, mp_limb_t);
            pe[0] = (mp_limb_t)primes.pri[i] * primes.pri[i];
            pe[0] *= primes.pri[i];
            lmmp_num_heap_push_(&heap, pe, 1);
        } else if (e == 4) {
            mp_ptr pe = ALLOC_TYPE(1, mp_limb_t);
            pe[0] = (mp_limb_t)primes.pri[i] * primes.pri[i];
            pe[0] *= pe[0];
            lmmp_num_heap_push_(&heap, pe, 1);
        } else if (e < 20) {
            mp_size_t pen = lmmp_pow_1_size_(primes.pri[i], e);
            mp_ptr pe = ALLOC_TYPE(pen, mp_limb_t);
            pen = 1;
            pe[0] = (mp_limb_t)primes.pri[i] * primes.pri[i];
            for (uint j = 2; j < e; ++j) {
                pe[pen] = lmmp_mul_1_(pe, pe, pen, primes.pri[i]);
                ++pen;
                pen -= pe[pen - 1] == 0 ? 1 : 0;
            }
            lmmp_num_heap_push_(&heap, pe, pen);
        } else {
            mp_size_t pen = lmmp_pow_1_size_(primes.pri[i], e);
            mp_ptr pe = ALLOC_TYPE(pen, mp_limb_t);
            pen = lmmp_pow_1_(pe, pen, primes.pri[i], e);
            lmmp_num_heap_push_(&heap, pe, pen);
        }
    }
    lmmp_prime_short_free_(&primes);

    num_node numa, numb;
    numa.num = NULL;
    numb.num = NULL;
    numa.n = 0;
    numb.n = 0;

    while (lmmp_num_heap_pop_(&heap, &numa)) {
        if (!lmmp_num_heap_pop_(&heap, &numb)) {
            break;
        }
        mp_ptr prod = ALLOC_TYPE(numa.n + numb.n, mp_limb_t);
        lmmp_mul_(prod, numb.num, numb.n, numa.num, numa.n);
        FREE(numa.num);
        FREE(numb.num);
        numa.num = prod;
        numa.n += numb.n;
        numa.n -= numa.num[numa.n - 1] == 0 ? 1 : 0;
        lmmp_num_heap_push_(&heap, numa.num, numa.n);
    }

    /* 乘以 2 的幂次方 */
    /* sum( floor(n / 2^k) ) = n - popcnt(n) */
    rn = n - lmmp_limb_popcnt_(n);

    mp_size_t sh_w = rn / LIMB_BITS;
    rn %= LIMB_BITS;
    lmmp_zero(dst, sh_w);
    mp_limb_t c = lmmp_shl_(dst + sh_w, numa.num, numa.n, rn);
    if (c) {
        dst[sh_w + numa.n] = c;
        sh_w += numa.n + 1;
    } else {
        sh_w += numa.n;
    }
    FREE(numa.num);
    lmmp_num_heap_free_(&heap);
    return sh_w;
}

mp_size_t lmmp_factorial_(mp_ptr dst, mp_size_t rn, uint n) {
    if (n <= 20) {
        rn = 1;
        for (uint i = 1; i <= n; ++i) {
            dst[0] = rn * i;
            rn = dst[0];
        }
        return 1;
    } else if (n <= 0xffff) {
        return lmmp_factorial_short_(dst, rn, n);
    } else {
        pri_int primes;
        lmmp_prime_int_init_(&primes, n);
        num_heap heap;
        lmmp_num_heap_init_(&heap, primes.prin);

        mp_size_t mpn = 1;
        mp_ptr mp = ALLOC_TYPE(FACTORIAL_MUL_MAX_THRESHOLD, mp_limb_t);
        mp[0] = 1;
        /* 跳过质数 2 */
        for (ulong i = 1; i < primes.prin; ++i) {
            ulong pn = n;
            ulong e = 0;
            while (pn > 0) {
                pn /= primes.pri[i];
                e += pn;
            }
            if (e >= FACTORIAL_PRIME_POW_THRESHOLD) {
                mp_size_t pon_max = lmmp_pow_1_size_(primes.pri[i], e);
                mp_ptr po = ALLOC_TYPE(pon_max, mp_limb_t);
                mp_size_t pon = lmmp_pow_1_(po, pon_max, primes.pri[i], e);
                lmmp_debug_assert(pon <= pon_max);
                lmmp_num_heap_push_(&heap, po, pon);
                continue;
            } else if (e > 2) {
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

            if (e == 1) {
                mp[mpn] = lmmp_mul_1_(mp, mp, mpn, primes.pri[i]);
                ++mpn;
                mpn -= mp[mpn - 1] == 0 ? 1 : 0;
            } else {
                mp[mpn] = lmmp_mul_1_(mp, mp, mpn, (mp_limb_t)(primes.pri[i]) * primes.pri[i]);
                ++mpn;
                mpn -= mp[mpn - 1] == 0 ? 1 : 0;
            }
            if (mpn == FACTORIAL_MUL_MAX_THRESHOLD) {
                lmmp_num_heap_push_(&heap, mp, mpn);
                mp = ALLOC_TYPE(FACTORIAL_MUL_MAX_THRESHOLD, mp_limb_t);
                mpn = 1;
                mp[0] = 1;
            }
        }
        if ( !(mpn == 1 && mp[0] == 1))
            lmmp_num_heap_push_(&heap, mp, mpn);

        lmmp_prime_int_free_(&primes);

        num_node numa, numb;
        numa.num = NULL;
        numb.num = NULL;
        numa.n = 0;
        numb.n = 0;

        while (lmmp_num_heap_pop_(&heap, &numa)) {
            if (!lmmp_num_heap_pop_(&heap, &numb)) {
                break;
            }
            mp_ptr prod = ALLOC_TYPE(numa.n + numb.n, mp_limb_t);
            lmmp_mul_(prod, numb.num, numb.n, numa.num, numa.n);
            FREE(numa.num);
            FREE(numb.num);
            numa.num = prod;
            numa.n += numb.n;
            numa.n -= prod[numa.n - 1] == 0 ? 1 : 0;
            lmmp_num_heap_push_(&heap, numa.num, numa.n);
        }

        /* 乘以 2 的幂次方 */
        /* sum( floor(n / 2^k) ) = n - popcnt(n) */
        rn = n - lmmp_limb_popcnt_(n);
        
        mp_size_t sh_w = rn / LIMB_BITS;
        rn %= LIMB_BITS;
        lmmp_zero(dst, sh_w);
        dst[sh_w + numa.n] = lmmp_shl_(dst + sh_w, numa.num, numa.n, rn);
        sh_w += numa.n + 1;
        sh_w -= dst[sh_w - 1] == 0 ? 1 : 0;

        FREE(numa.num);
        lmmp_num_heap_free_(&heap);
        return sh_w;
    }
}