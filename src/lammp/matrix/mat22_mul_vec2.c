#include "../../../include/lammp/lmmpn.h"
#include "../../../include/lammp/matrix.h"
#include "../../../include/lammp/impl/signed.h"

#define MUL(tp, tn, a, na, b, nb)        \
    if (na == 0 || nb == 0) {            \
        tp[0] = 0;                       \
        tn = 0;                          \
    } else {                             \
        if (na >= nb)                    \
            lmmp_mul_(tp, a, na, b, nb); \
        else                             \
            lmmp_mul_(tp, b, nb, a, na); \
        tn = na + nb;                    \
        tn -= tp[tn - 1] == 0;           \
    }

void lmmp_mat22_mul_vec2_(lmmp_vec2_t* dst, const lmmp_mat22_t* mat, const lmmp_vec2_t* vec) {
    TEMP_DECL;
    mp_ssize_t n00 = LMMP_ABS(mat->n00);
    mp_ssize_t n01 = LMMP_ABS(mat->n01);
    mp_ssize_t n10 = LMMP_ABS(mat->n10);
    mp_ssize_t n11 = LMMP_ABS(mat->n11);
    mp_ssize_t vn0 = LMMP_ABS(vec->n0);
    mp_ssize_t vn1 = LMMP_ABS(vec->n1);
    mp_ssize_t tn1 = LMMP_MAX(n00, n01);
    tn1 = LMMP_MAX(tn1, n10);
    tn1 = LMMP_MAX(tn1, n11);
    mp_ssize_t tn2 = tn1 + LMMP_MAX(vn0, vn1);
    mp_ssize_t tn3 = tn2;
    mp_ptr tp1 = TALLOC_TYPE(tn2 + 1, mp_limb_t);
    mp_ptr tp2 = TALLOC_TYPE(tn2 + 1, mp_limb_t);
    mp_ptr tp3 = dst->a1;
    if (dst == vec) 
        tp3 = TALLOC_TYPE(tn2 + 1, mp_limb_t);

    MUL(tp1, tn1, mat->a00, n00, vec->a0, vn0)
    MUL(tp2, tn2, mat->a01, n01, vec->a1, vn1)
    tn1 = ((mat->n00 > 0) ^ (vec->n0 > 0)) ? -tn1 : tn1;
    tn2 = ((mat->n01 > 0) ^ (vec->n1 > 0)) ? -tn2 : tn2;
    
    tn1 = lmmp_add_signed_(tp1, tp1, tn1, tp2, tn2);

    MUL(tp2, tn2, mat->a10, n10, vec->a0, vn0)
    MUL(tp3, tn3, mat->a11, n11, vec->a1, vn1)
    tn2 = ((mat->n10 > 0) ^ (vec->n0 > 0)) ? -tn2 : tn2;
    tn3 = ((mat->n11 > 0) ^ (vec->n1 > 0)) ? -tn3 : tn3;
    
    tn3 = lmmp_add_signed_(tp3, tp2, tn2, tp3, tn3);

    dst->n0 = tn1;
    dst->n1 = tn3;
    tn1 = LMMP_ABS(tn1);
    tn3 = LMMP_ABS(tn3);
    lmmp_copy(dst->a0, tp1, tn1);
    lmmp_copy(dst->a1, tp3, tn3);

    TEMP_FREE;
}
