#include "../../../include/lammp/matrix.h"
#include "../../../include/lammp/lmmpn.h"
#include "../../../include/lammp/impl/heap.h"

#define COPY(tp, src, n)           \
    tp = ALLOC_TYPE(n, mp_limb_t); \
    lmmp_copy(tp, src, n)

#define POP_THRESHOLD 5

mp_ssize_t lmmp_vec_elem_mul_(mp_ptr* dst, const lmmp_vecn_t* vec) {
    lmmp_debug_assert(dst != NULL && vec != NULL);
    lmmp_debug_assert(vec->n > 0);

    num_heap heap;
    lmmp_num_heap_init_(&heap, vec->n);

    mp_size_t mpn = 1, len;
    mp_ptr tp;
    mp_ptr mp1 = ALLOC_TYPE(VEC_ELEMMUL_MP_THRESHOLD, mp_limb_t);
    mp_ptr mp2 = ALLOC_TYPE(VEC_ELEMMUL_MP_THRESHOLD, mp_limb_t);
    mp1[0] = 1;
    bool imp = true; // true 表示当前mp1存储连乘结果

    bool sign = true;
    for (mat_size_t i = 0; i < vec->n; ++i) {
        if (vec->num[i] == NULL || vec->len[i] == 0) {
            *dst = NULL;
            return 0;
        } else if (vec->len[i] < 0) {
            sign = !sign;
        }
        len = LMMP_ABS(vec->len[i]);
        if (len > VEC_ELEMMUL_MP_THRESHOLD || (len + mpn > VEC_ELEMMUL_MP_THRESHOLD)) {
            COPY(tp, vec->num[i], len);
            lmmp_num_heap_push_(&heap, tp, len);
        } else {
            if (imp) {
                if (mpn >= len)
                    lmmp_mul_basecase_(mp2, mp1, mpn, vec->num[i], len);
                else
                    lmmp_mul_basecase_(mp2, vec->num[i], len, mp1, mpn);
                mpn += len;
                mpn -= mp2[mpn - 1] == 0 ? 1 : 0;
                imp = false;
            } else {
                if (mpn >= len)
                    lmmp_mul_basecase_(mp1, mp2, mpn, vec->num[i], len);
                else
                    lmmp_mul_basecase_(mp1, vec->num[i], len, mp2, mpn);
                mpn += len;
                mpn -= mp1[mpn - 1] == 0 ? 1 : 0;
                imp = true;
            }
        }
        if (mpn + POP_THRESHOLD > VEC_ELEMMUL_MP_THRESHOLD) {
            if (imp) {
                lmmp_num_heap_push_(&heap, mp1, mpn);
                mp1 = ALLOC_TYPE(VEC_ELEMMUL_MP_THRESHOLD, mp_limb_t);
                mpn = 1;
                mp1[0] = 1;
            } else {
                lmmp_num_heap_push_(&heap, mp2, mpn);
                mp2 = ALLOC_TYPE(VEC_ELEMMUL_MP_THRESHOLD, mp_limb_t);
                mpn = 1;
                mp2[0] = 1;
            }
        }
    }
    if (imp) {
        if (!(mpn == 1 && mp1[0] == 1))
            lmmp_num_heap_push_(&heap, mp1, mpn);
        else
            lmmp_free(mp1);
        lmmp_free(mp2);
    } else {
        if (!(mpn == 1 && mp2[0] == 1))
            lmmp_num_heap_push_(&heap, mp2, mpn);
        else
            lmmp_free(mp2);
        lmmp_free(mp1);
    }

    tp = lmmp_num_heap_mul_(&heap, &mpn);
    lmmp_num_heap_free_(&heap);
    *dst = tp;
    return sign ? mpn : -mpn;
}

mp_size_t lmmp_limb_elem_mul_(mp_ptr* dst, const mp_limb_t* limb, mp_size_t n) {
    lmmp_debug_assert(dst != NULL && limb != NULL);
    lmmp_debug_assert(n > 0);

    num_heap heap;
#define heap_size (n / LIMB_ELEMMUL_MP_THRESHOLD)
    lmmp_num_heap_init_(&heap, LMMP_MAX(heap_size, 4));
#undef heap_size

    mp_size_t mpn = 1;
    mp_ptr mp = ALLOC_TYPE(LIMB_ELEMMUL_MP_THRESHOLD, mp_limb_t);
    mp[0] = 1;
    for (mat_size_t i = 0; i < n; ++i) {
        if (limb[i] == 0) {
            *dst = NULL;
            return 0;
        }
        mp[mpn] = lmmp_mul_1_(mp, mp, mpn, limb[i]);
        ++mpn;
        mpn -= mp[mpn - 1] == 0 ? 1 : 0;
        if (mpn == LIMB_ELEMMUL_MP_THRESHOLD) {
            lmmp_num_heap_push_(&heap, mp, mpn);
            mp = ALLOC_TYPE(LIMB_ELEMMUL_MP_THRESHOLD, mp_limb_t);
            mpn = 1;
            mp[0] = 1;
        }
    }

    if (!(mpn == 1 && mp[0] == 1))
        lmmp_num_heap_push_(&heap, mp, mpn);
    else
        lmmp_free(mp);

    mp = lmmp_num_heap_mul_(&heap, &mpn);
    lmmp_num_heap_free_(&heap);
    *dst = mp;
    return mpn;
}

mp_ssize_t lmmp_slimb_elem_mul_(mp_ptr* dst, const mp_slimb_t* slimb, mp_size_t n) {
    lmmp_debug_assert(dst != NULL && slimb != NULL);
    lmmp_debug_assert(n > 0);

    num_heap heap;
#define heap_size (n / LIMB_ELEMMUL_MP_THRESHOLD)
    lmmp_num_heap_init_(&heap, LMMP_MAX(heap_size, 4));
#undef heap_size

    mp_size_t mpn = 1;
    mp_ptr mp = ALLOC_TYPE(LIMB_ELEMMUL_MP_THRESHOLD, mp_limb_t);
    mp[0] = 1;
    bool sign = true;
    for (mat_size_t i = 0; i < n; ++i) {
        if (slimb[i] == 0) {
            *dst = NULL;
            return 0;
        } else if (slimb[i] < 0) {
            sign = !sign;
            mp[mpn] = lmmp_mul_1_(mp, mp, mpn, -slimb[i]);
        } else {
            mp[mpn] = lmmp_mul_1_(mp, mp, mpn, slimb[i]);
        }
        ++mpn;
        mpn -= mp[mpn - 1] == 0 ? 1 : 0;
        if (mpn == LIMB_ELEMMUL_MP_THRESHOLD) {
            lmmp_num_heap_push_(&heap, mp, mpn);
            mp = ALLOC_TYPE(LIMB_ELEMMUL_MP_THRESHOLD, mp_limb_t);
            mpn = 1;
            mp[0] = 1;
        }
    }

    if (!(mpn == 1 && mp[0] == 1))
        lmmp_num_heap_push_(&heap, mp, mpn);
    else
        lmmp_free(mp);

    mp = lmmp_num_heap_mul_(&heap, &mpn);
    lmmp_num_heap_free_(&heap);
    *dst = mp;
    return sign ? mpn : -mpn;
}