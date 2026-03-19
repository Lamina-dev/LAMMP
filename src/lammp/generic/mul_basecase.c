#include "../../../include/lammp/lmmpn.h"

void lmmp_sqr_basecase_(mp_ptr restrict dst, mp_srcptr restrict numa, mp_size_t na) {
    mp_size_t nb = na;
    mp_srcptr numb = numa;
    dst[na] = lmmp_mul_1_(dst, numa, na, numb[0]);
    while (--nb >= 1) {
        ++dst;
        ++numb;
        dst[na] = lmmp_addmul_1_(dst, numa, na, numb[0]);
    }
}

void lmmp_mul_basecase_(mp_ptr restrict dst, mp_srcptr restrict numa, mp_size_t na, mp_srcptr restrict numb, mp_size_t nb) {
    lmmp_param_assert(na >= nb);
    lmmp_param_assert(nb >= 1);
    dst[na] = lmmp_mul_1_(dst, numa, na, numb[0]);
    while (--nb >= 1) {
        ++dst;
        ++numb;
        dst[na] = lmmp_addmul_1_(dst, numa, na, numb[0]);
    }
}
