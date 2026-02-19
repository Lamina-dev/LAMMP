#include "../../../include/lammp/impl/heap.h"
#include "../../../include/lammp/impl/prime_table.h"

mp_size_t lmmp_nPr_short_(mp_ptr dst, mp_size_t rn, ulong n, ulong r) {
    if (n <= 20) {
        static const ulong factorial[21] = {1,
                                            1,
                                            2,
                                            6,
                                            24,
                                            120,
                                            720,
                                            5040,
                                            40320,
                                            362880,
                                            3628800,
                                            39916800,
                                            479001600,
                                            6227020800,
                                            87178291200,
                                            1307674368000,
                                            20922789888000,
                                            355687428096000,
                                            6402373705728000,
                                            121645100408832000,
                                            2432902008176640000};
        dst[0] = factorial[n] / factorial[n - r];
        return 1;
    } else if (r <= 10) {
        dst[0] = 1;
        rn = 1;
        ulong t = 0;
        ulong i = n - r + 1;
        for (; i <= (ulong)n - 3; i += 3) {
            t = i * (i + 1) * (i + 2);
            dst[rn] = lmmp_mul_1_(dst, dst, rn, t);
            ++rn;
            rn -= dst[rn - 1] == 0 ? 1 : 0;
        }
        t = 1;
        for (; i <= n; ++i) {
            t *= i;
        }
        dst[rn] = lmmp_mul_1_(dst, dst, rn, t);
        ++rn;
        rn -= dst[rn - 1] == 0 ? 1 : 0;
        return rn;
    } else if (n <= 0xff) {
        dst[0] = n - r + 1;
        rn = 1;
        ulong t = 0;
        ulong i = n - r + 2;
        lmmp_debug_assert(n >= 7);
        for (; i <= (ulong)n - 7; i += 7) {
            t = i * (i + 1) * (i + 2) * (i + 3) * (i + 4) * (i + 5) * (i + 6);
            dst[rn] = lmmp_mul_1_(dst, dst, rn, t);
            ++rn;
            rn -= dst[rn - 1] == 0 ? 1 : 0;
        }
        t = 1;
        for (; i <= n; ++i) {
            t *= i;
        }
        dst[rn] = lmmp_mul_1_(dst, dst, rn, t);
        ++rn;
        rn -= dst[rn - 1] == 0 ? 1 : 0;
        return rn;
    } else if (n <= 0xfff && rn < PERMUTATION_RN_BASECASE_THRESHOLD) {
        dst[0] = n - r + 1;
        rn = 1;
        ulong t = 0;
        ulong i = n - r + 2;
        for (; i <= (ulong)n - 4; i += 4) {
            t = i * (i + 1) * (i + 2) * (i + 3);
            dst[rn] = lmmp_mul_1_(dst, dst, rn, t);
            ++rn;
            rn -= dst[rn - 1] == 0 ? 1 : 0;
        }
        t = 1;
        for (; i <= n; ++i) {
            t *= i;
        }
        dst[rn] = lmmp_mul_1_(dst, dst, rn, t);
        ++rn;
        rn -= dst[rn - 1] == 0 ? 1 : 0;
        return rn;
    } else {
        lmmp_debug_assert(n <= 0xffff);

        num_heap heap;
#define heap_size (r / PERMUTATION_MUL_MAX_THRESHOLD)
        lmmp_num_heap_init_(&heap, LMMP_MAX(heap_size, 4));
#undef heap_size

        mp_size_t mpn = 1;
        mp_ptr mp = ALLOC_TYPE(PERMUTATION_MUL_MAX_THRESHOLD, mp_limb_t);
        mp[0] = 1;
        ulong i = n - r + 1;
        ulong t = 1;
        for (; i <= (ulong)n - 3; i += 3) {
            t = i * (i + 1) * (i + 2);
            mp[mpn] = lmmp_mul_1_(mp, mp, mpn, t);
            ++mpn;
            mpn -= mp[mpn - 1] == 0 ? 1 : 0;
            if (mpn == PERMUTATION_MUL_MAX_THRESHOLD) {
                lmmp_num_heap_push_(&heap, mp, mpn);
                mp = ALLOC_TYPE(PERMUTATION_MUL_MAX_THRESHOLD, mp_limb_t);
                mpn = 1;
                mp[0] = 1;
            }
        }
        t = 1;
        for (; i <= n; ++i) {
            t *= i;
        }

        if (!(mpn == 1 && mp[0] == 1))
            lmmp_num_heap_push_(&heap, mp, mpn);

        mp = lmmp_num_heap_mul_(&heap, &mpn);
        
        dst[mpn] = lmmp_mul_1_(dst, mp, mpn, t);
        rn = dst[mpn] == 0 ? mpn : mpn + 1;
        lmmp_free(mp);
        lmmp_num_heap_free_(&heap);
        return rn;
    }
}

mp_size_t lmmp_nPr_int_(mp_ptr dst, mp_size_t rn, ulong n, ulong r) {
    if (rn < PERMUTATION_RN_BASECASE_THRESHOLD) {
        if (r <= 10 || n >= 0x10000000) {
            dst[0] = 1;
            rn = 1;
            for (ulong i = n - r + 1; i <= n; ++i) {
                dst[rn] = lmmp_mul_1_(dst, dst, rn, i);
                ++rn;
                rn -= dst[rn - 1] == 0 ? 1 : 0;
            }
            return rn;
        } else {
            dst[0] = n - r + 1;
            rn = 1;
            ulong t = 0;
            ulong i = n - r + 2;
            for (; i <= (ulong)n - 2; i += 2) {
                t = i * (i + 1);
                dst[rn] = lmmp_mul_1_(dst, dst, rn, t);
                ++rn;
                rn -= dst[rn - 1] == 0 ? 1 : 0;
            }
            t = 1;
            for (; i <= n; ++i) {
                t *= i;
            }
            dst[rn] = lmmp_mul_1_(dst, dst, rn, t);
            ++rn;
            rn -= dst[rn - 1] == 0 ? 1 : 0;
            return rn;
        }
    } else if (rn < PERMUTATION_RN_MUL_THRESHOLD) {
        num_heap heap;
#define heap_size (r / PERMUTATION_MUL_MAX_THRESHOLD)
        lmmp_num_heap_init_(&heap, LMMP_MAX(heap_size, 4));
#undef heap_size

        mp_size_t mpn = 1;
        mp_ptr mp = ALLOC_TYPE(PERMUTATION_MUL_MAX_THRESHOLD, mp_limb_t);
        mp[0] = 1;
        for (ulong i = n - r + 1; i <= n; ++i) {
            mp[mpn] = lmmp_mul_1_(mp, mp, mpn, i);
            ++mpn;
            mpn -= mp[mpn - 1] == 0 ? 1 : 0;
            if (mpn == PERMUTATION_MUL_MAX_THRESHOLD) {
                lmmp_num_heap_push_(&heap, mp, mpn);
                mp = ALLOC_TYPE(PERMUTATION_MUL_MAX_THRESHOLD, mp_limb_t);
                mpn = 1;
                mp[0] = 1;
            }
        }
        if (!(mpn == 1 && mp[0] == 1))
            lmmp_num_heap_push_(&heap, mp, mpn);

        mp = lmmp_num_heap_mul_(&heap, &mpn);

        lmmp_num_heap_free_(&heap);
        lmmp_copy(dst, mp, mpn);
        
        lmmp_free(mp);

        return mpn;
    } else {
        lmmp_debug_assert(n <= 0xffffffff);

        pri_int primes;
        lmmp_prime_int_init_(&primes, n);
        num_heap heap;
        lmmp_num_heap_init_(&heap, primes.prin);

        r = n - r;

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
            pn = r;
            while (pn > 0) {
                pn /= primes.pri[i];
                e -= pn;
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

        lmmp_prime_int_free_(&primes);

        mp = lmmp_num_heap_mul_(&heap, &mpn);
        lmmp_num_heap_free_(&heap);

        /* 乘以 2 的幂次方 */
        rn = n - lmmp_limb_popcnt_(n);
        rn -= r - lmmp_limb_popcnt_(r);

        mp_size_t sh_w = rn / LIMB_BITS;
        rn %= LIMB_BITS;
        lmmp_zero(dst, sh_w);
        dst[sh_w + mpn] = lmmp_shl_(dst + sh_w, mp, mpn, rn);
        sh_w += mpn + 1;
        sh_w -= dst[sh_w - 1] == 0 ? 1 : 0;

        lmmp_free(mp);
        return sh_w;
    }
}

mp_size_t lmmp_nPr_long_(mp_ptr dst, mp_size_t rn, ulong n, ulong r) {
    if (rn < PERMUTATION_RN_BASECASE_THRESHOLD) {
        if (n == 0xffffffffffffffff || r <= 3) {
            dst[0] = 1;
            rn = 1;
            for (ulong i = n - r + 1; i != 0 && i <= n; ++i) {
                dst[rn] = lmmp_mul_1_(dst, dst, rn, i);
                ++rn;
                rn -= dst[rn - 1] == 0 ? 1 : 0;
            }
            return rn;
        } else {
            dst[0] = n - r + 1;
            rn = 1;
            for (ulong i = n - r + 2; i <= n; ++i) {
                dst[rn] = lmmp_mul_1_(dst, dst, rn, i);
                ++rn;
                rn -= dst[rn - 1] == 0 ? 1 : 0;
            }
            return rn;
        }
    } else {
        num_heap heap;
#define heap_size (r / PERMUTATION_MUL_MAX_THRESHOLD)
        lmmp_num_heap_init_(&heap, LMMP_MAX(heap_size, 4));
#undef heap_size

        mp_size_t mpn = 1;
        mp_ptr mp = ALLOC_TYPE(PERMUTATION_MUL_MAX_THRESHOLD, mp_limb_t);
        mp[0] = 1;
        for (ulong i = n - r + 1; i <= n; ++i) {
            mp[mpn] = lmmp_mul_1_(mp, mp, mpn, i);
            ++mpn;
            mpn -= mp[mpn - 1] == 0 ? 1 : 0;
            if (mpn == PERMUTATION_MUL_MAX_THRESHOLD) {
                lmmp_num_heap_push_(&heap, mp, mpn);
                mp = ALLOC_TYPE(PERMUTATION_MUL_MAX_THRESHOLD, mp_limb_t);
                mpn = 1;
                mp[0] = 1;
            }
        }

        if (!(mpn == 1 && mp[0] == 1))
            lmmp_num_heap_push_(&heap, mp, mpn);

        mp = lmmp_num_heap_mul_(&heap, &mpn);

        lmmp_copy(dst, mp, mpn);
        lmmp_free(mp);
        lmmp_num_heap_free_(&heap);
        return mpn;
    }
}
