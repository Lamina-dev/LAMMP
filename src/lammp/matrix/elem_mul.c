#include "../../../include/lammp/matrix.h"
#include "../../../include/lammp/lmmpn.h"
#include "../../../include/lammp/impl/heap.h"

mp_ssize_t lmmp_vec_elem_mul_(mp_ptr* dst, lmmp_vecn_t* vec) {
    
}

mp_size_t lmmp_limb_elem_mul_(mp_ptr* dst, mp_limb_t* limb, mp_size_t n) {
    lmmp_debug_assert(dst != NULL && limb != NULL);
    lmmp_debug_assert(n > 0);

    num_heap heap;
#define heap_size (n / ELEMMUL_MP_THRESHOLD)
    lmmp_num_heap_init_(&heap, LMMP_MAX(heap_size, 4));
#undef heap_size

    mp_size_t mpn = 1;
    mp_ptr mp = ALLOC_TYPE(ELEMMUL_MP_THRESHOLD, mp_limb_t);
    mp[0] = 1;
    for (mat_size_t i = 0; i < n; ++i) {
        if (limb[i] == 0) {
            *dst = NULL;
            return 0;
        }
        mp[mpn] = lmmp_mul_1_(mp, mp, mpn, limb[i]);
        ++mpn;
        mpn -= mp[mpn - 1] == 0 ? 1 : 0;
        if (mpn == ELEMMUL_MP_THRESHOLD) {
            lmmp_num_heap_push_(&heap, mp, mpn);
            mp = ALLOC_TYPE(ELEMMUL_MP_THRESHOLD, mp_limb_t);
            mpn = 1;
            mp[0] = 1;
        }
    }

    if (!(mpn == 1 && mp[0] == 1))
        lmmp_num_heap_push_(&heap, mp, mpn);

    mp = lmmp_num_heap_mul_(&heap, &mpn);
    lmmp_num_heap_free_(&heap);
    *dst = mp;
    return mpn;
}

mp_ssize_t lmmp_slimb_elem_mul_(mp_ptr* dst, mp_slimb_t* slimb, mp_size_t n) {
    lmmp_debug_assert(dst != NULL && slimb != NULL);
    lmmp_debug_assert(n > 0);

    num_heap heap;
#define heap_size (n / ELEMMUL_MP_THRESHOLD)
    lmmp_num_heap_init_(&heap, LMMP_MAX(heap_size, 4));
#undef heap_size

    mp_ssize_t mpn = 1;
    mp_ptr mp = ALLOC_TYPE(ELEMMUL_MP_THRESHOLD, mp_limb_t);
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
        if (mpn == ELEMMUL_MP_THRESHOLD) {
            lmmp_num_heap_push_(&heap, mp, mpn);
            mp = ALLOC_TYPE(ELEMMUL_MP_THRESHOLD, mp_limb_t);
            mpn = 1;
            mp[0] = 1;
        }
    }

    if (!(mpn == 1 && mp[0] == 1))
        lmmp_num_heap_push_(&heap, mp, mpn);

    mp = lmmp_num_heap_mul_(&heap, &mpn);
    lmmp_num_heap_free_(&heap);
    *dst = mp;
    return  sign ? mpn : -mpn;
}