#include "../../include/lammp/lmmpn.h"

mp_limb_t lmmp_div_basecase_(mp_ptr dstq, mp_ptr numa, mp_size_t na, mp_srcptr numb, mp_size_t nb, mp_limb_t inv21) {
    lmmp_debug_assert(na >= nb);
    lmmp_debug_assert(nb >= 3);
    lmmp_debug_assert(numb[nb - 1] >= 0x8000000000000000);
    mp_size_t nq = na - nb;

    numa += na;

    mp_limb_t qh = lmmp_cmp_(numa - nb, numb, nb) >= 0;
    if (qh) {
        lmmp_sub_n_(numa - nb, numa - nb, numb, nb);
    }

    nb -= 2;
    numa -= 2;

    mp_limb_t d1 = numb[nb + 1], d0 = numb[nb + 0];

    while (nq) {
        mp_limb_t q;
        --numa;
        if (numa[2] == d1 && numa[1] == d0) {
            q = LIMB_MAX;
            lmmp_submul_1_(numa - nb, numb, nb + 2, q);
        } else {
            mp_limb_t cy, cy1;
            q = lmmp_div_3_2_(numa, numb + nb, inv21);
            cy = lmmp_submul_1_(numa - nb, numb, nb, q);
            cy1 = numa[0] < cy;
            numa[0] -= cy;
            cy = numa[1] < cy1;
            numa[1] -= cy1;
            if (cy) {
                lmmp_add_n_(numa - nb, numa - nb, numb, nb + 2);
                --q;
            }
        }
        dstq[--nq] = q;
    }
    return qh;
}
