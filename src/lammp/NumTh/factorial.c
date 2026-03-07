#include "../../../include/lammp/impl/heap.h"
#include "../../../include/lammp/impl/prime_table.h"
#include "../../../include/lammp/lmmpn.h"
#include "../../../include/lammp/numth.h"

#define MUL(dst, ap, an, bp, bn)        \
    if (an >= bn)                       \
        lmmp_mul_(dst, ap, an, bp, bn); \
    else                                \
        lmmp_mul_(dst, bp, bn, ap, an)

typedef struct factor {
    uint f;
    uint j;
} factor;

typedef struct factor* factors;

mp_size_t lmmp_fac_j_(mp_ptr dst, mp_size_t rn, const factors fac, uint nfactors, uint N) {
    if (nfactors <= 20) {
        dst[0] = 1;
        rn = 1;
        for (uint i = 0; i < nfactors; i++) {
            ulong f = fac[i].f;
            if (fac[i].j == 1) {
                dst[rn] = lmmp_mul_1_(dst, dst, rn, f);
                ++rn;
                rn -= dst[rn - 1] == 0;
            } else if (fac[i].j == 2) {
                dst[rn] = lmmp_mul_1_(dst, dst, rn, f * f);
                ++rn;
                rn -= dst[rn - 1] == 0;
            } else if (fac[i].j == 3) {
                lmmp_debug_assert(f <= 0xffff);
                dst[rn] = lmmp_mul_1_(dst, dst, rn, f * f * f);
                ++rn;
                rn -= dst[rn - 1] == 0;
            } else if (fac[i].j == 4) {
                lmmp_debug_assert(f <= 0xffff);
                f = f * f;
                dst[rn] = lmmp_mul_1_(dst, dst, rn, f * f);
                ++rn;
                rn -= dst[rn - 1] == 0;
            } else {
                lmmp_debug_assert(f <= 0xffff);
                ulong p2 = f * f;
                ulong p4 = p2 * p2;
                for (uint j = 0; j < fac[i].j - 3; j += 4) {
                    dst[rn] = lmmp_mul_1_(dst, dst, rn, p4);
                    ++rn;
                    rn -= dst[rn - 1] == 0;
                }
                switch (fac[i].j % 4) {
                    case 1:
                        dst[rn] = lmmp_mul_1_(dst, dst, rn, fac[i].f);
                        ++rn;
                        rn -= dst[rn - 1] == 0;
                        break;
                    case 2:
                        dst[rn] = lmmp_mul_1_(dst, dst, rn, p2);
                        ++rn;
                        rn -= dst[rn - 1] == 0;
                        break;
                    case 3:
                        dst[rn] = lmmp_mul_1_(dst, dst, rn, p2 * fac[i].f);
                        ++rn;
                        rn -= dst[rn - 1] == 0;
                        break;
                    default:
                        break;
                }
            }
        }
        return rn;
    } else {
        TEMP_B_DECL;
        mp_size_t new_fac_cap = lmmp_get_prime_count_table(N / 2) + 1;
        factors new_fac = BALLOC_TYPE(new_fac_cap, factor);
        mp_size_t new_nfactors = 0;

        num_heap heap;
        // heap size 除以2是因为不超过N/2到N的整数个数最多为N/2，而又必须要是质数，偶数和3的倍数分别占据
        // 1/2 和 1/3 的个数，即质数只占据 (1-1/2)(1-1/3)=1/3 的个数，所以除以3 
        // 又除以2是因为因子都小于2^32，64位可以存放两个因子的乘积。
#define heap_size ((N / 4 / 3) / PERMUTATION_MUL_MAX_THRESHOLD)
        lmmp_num_heap_init_(&heap, heap_size + 4);
#undef heap_size

        mp_size_t mpn = 1;
        mp_ptr mp = ALLOC_TYPE(PERMUTATION_MUL_MAX_THRESHOLD, mp_limb_t);
        mp[0] = 1;
        ulong ulongt = 1;
        for (mp_size_t i = 0; i < nfactors; i++) {
            lmmp_debug_assert(fac[i].j != 0);
            if (fac[i].j > 1) {
                new_fac[new_nfactors].f = fac[i].f;
                new_fac[new_nfactors++].j = fac[i].j >> 1;
                lmmp_debug_assert(new_nfactors <= new_fac_cap);
            } 
            if (fac[i].j & 1) {
                ulongt *= fac[i].f;
                if (ulongt >= 0xffffffff) {
                    mp[mpn] = lmmp_mul_1_(mp, mp, mpn, ulongt);
                    ulongt = 1;
                    ++mpn;
                    mpn -= mp[mpn - 1] == 0 ? 1 : 0;
                    if (mpn == PERMUTATION_MUL_MAX_THRESHOLD) {
                        lmmp_num_heap_push_(&heap, mp, mpn);
                        mp = ALLOC_TYPE(PERMUTATION_MUL_MAX_THRESHOLD, mp_limb_t);
                        mpn = 1;
                        mp[0] = 1;
                    }
                }
            }
        }
        if (ulongt != 1) {
            mp[mpn] = lmmp_mul_1_(mp, mp, mpn, ulongt);
            ulongt = 1;
            ++mpn;
            mpn -= mp[mpn - 1] == 0 ? 1 : 0;
        }
        if (!(mpn == 1 && mp[0] == 1))
            lmmp_num_heap_push_(&heap, mp, mpn);
        else
            lmmp_free(mp);
        mp = lmmp_num_heap_mul_(&heap, &mpn);
        lmmp_num_heap_free_(&heap);

        lmmp_assert(rn >= mpn);
        mp_size_t tn = ((rn - mpn) >> 1) + 1;
        
        mp_ptr tp = BALLOC_TYPE(3 * tn + 3, mp_limb_t);

        tn = lmmp_fac_j_(tp, tn, new_fac, new_nfactors, N / 2);

        mp_ptr tp2 = tp + tn + 1;
        lmmp_sqr_(tp2, tp, tn);
        tn <<= 1;
        tn -= tp2[tn - 1] == 0;
        
        MUL(dst, mp, mpn, tp2, tn);
        rn = tn + mpn;
        rn -= dst[rn - 1] == 0;
        
        lmmp_free(mp);
        TEMP_B_FREE;
        return rn;
    }
}

mp_size_t lmmp_fac_(mp_ptr dst, mp_size_t rn, uint n) {
    lmmp_prime_int_table_init_(n, true);
    TEMP_B_DECL;
    uint nfactors = lmmp_get_prime_count_table(n) - 1;
    factors fac = BALLOC_TYPE(nfactors, factor);
    for (uint i = 0; i < nfactors; i++) {
        fac[i].f = lmmp_get_nth_prime_table(i + 1);
        fac[i].j = 0;
        uint pn = n;
        uint e = 0;
        while (pn > 0) {
            pn /= fac[i].f;
            e += pn;
        }
        fac[i].j = e;
    }

    mp_size_t shl = n - lmmp_limb_popcnt_(n);
    mp_size_t shw = shl / LIMB_BITS;
    shl %= LIMB_BITS;

    lmmp_zero(dst, shw);
    rn = lmmp_fac_j_(dst + shw, rn - shw, fac, nfactors, n);

    dst[shw + rn] = lmmp_shl_(dst + shw, dst + shw, rn, shl);
    rn += shw + 1;
    rn -= dst[rn - 1] == 0 ? 1 : 0;

    TEMP_B_FREE;
    return rn;
}