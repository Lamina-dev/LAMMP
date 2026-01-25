#include "../../include/lammp/lmmpn.h"

// qh:[dstq,n]=[numa,2*n] div [numb,n], [numa,n]=[numa,2*n] mod [numb,n], return qh
// need(n>=6, MSB(numb)=1, [tp,n], inv21=(2^192-1)/[numb+nb-2,2]-2^64, sep(dstq,numa,numb,tp))
mp_limb_t lmmp_div_divide_n_(mp_ptr dstq, mp_ptr numa, mp_srcptr numb, mp_size_t n, mp_limb_t inv21, mp_ptr tp) {
    lmmp_assert(n >= 6);
    lmmp_assert(numb[n - 1] >= 0x8000000000000000);
    mp_size_t lo = n >> 1, hi = n - lo;
    mp_limb_t cy, qh, ql;

    if (hi < DIV_DIVIDE_THRESHOLD) {
        qh = lmmp_div_basecase_(dstq + lo, numa + 2 * lo, 2 * hi, numb + lo, hi, inv21);
    } else {
        qh = lmmp_div_divide_n_(dstq + lo, numa + 2 * lo, numb + lo, hi, inv21, tp);
    }
    lmmp_mul_(tp, dstq + lo, hi, numb, lo);

    cy = lmmp_sub_n_(numa + lo, numa + lo, tp, n);
    if (qh)
        cy += lmmp_sub_n_(numa + n, numa + n, numb, lo);

    while (cy) {
        qh -= lmmp_sub_1_(dstq + lo, dstq + lo, hi, 1);
        cy -= lmmp_add_n_(numa + lo, numa + lo, numb, n);
    }

    if (lo < DIV_DIVIDE_THRESHOLD) {
        ql = lmmp_div_basecase_(dstq, numa + hi, 2 * lo, numb + hi, lo, inv21);
    } else {
        ql = lmmp_div_divide_n_(dstq, numa + hi, numb + hi, lo, inv21, tp);
    }
    lmmp_mul_(tp, numb, hi, dstq, lo);

    cy = lmmp_sub_n_(numa, numa, tp, n);
    if (ql)
        cy += lmmp_sub_n_(numa + lo, numa + lo, numb, hi);

    while (cy) {
        lmmp_sub_1_(dstq, dstq, lo, 1);
        cy -= lmmp_add_n_(numa, numa, numb, n);
    }
    return qh;
}

mp_limb_t lmmp_div_divide_(mp_ptr dstq, mp_ptr numa, mp_size_t na, mp_srcptr numb, mp_size_t nb, mp_limb_t inv21) {
    lmmp_debug_assert(na >= 2 * nb);
    lmmp_debug_assert(nb >= 6);
    lmmp_debug_assert(numb[nb - 1] >= 0x8000000000000000ull);
    mp_size_t nq = na - nb;

    dstq += nq;
    numa += nq;

    do {
        nq -= nb;
    } while (nq >= nb);

    dstq -= nq;
    numa -= nq;

    /* Perform the typically smaller block first. */
    mp_limb_t qh = lmmp_div_s_(dstq, numa, nb + nq, numb, nb);

    TEMP_DECL;
    mp_ptr tp = TALLOC_TYPE(nb, mp_limb_t);
    nq = na - nb - nq;

    do {
        dstq -= nb;
        numa -= nb;
        lmmp_div_divide_n_(dstq, numa, numb, nb, inv21, tp);
        nq -= nb;
    } while (nq > 0);

    TEMP_FREE;
    return qh;
}
