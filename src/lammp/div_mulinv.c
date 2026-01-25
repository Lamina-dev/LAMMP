#include "../../include/lammp/lmmpn.h"

void lmmp_inv_prediv_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_size_t ni) {
    lmmp_debug_assert(na >= ni);
    lmmp_debug_assert(ni > 0);
    lmmp_debug_assert(numa[na - 1] >= 0x8000000000000000ull);
    TEMP_DECL;
    mp_limb_t cy;
    mp_ptr tp = TALLOC_TYPE(ni + 1, mp_limb_t);

    if (na == ni) {
        lmmp_copy(tp + 1, numa, ni);
        tp[0] = 1;
        cy = 0;
    } else {
        cy = lmmp_add_1_(tp, numa + na - (ni + 1), ni + 1, 1);
    }
    if (cy)
        lmmp_zero(dst, ni);
    else {
        mp_ptr invappr = TALLOC_TYPE(ni + 1, mp_limb_t);
        lmmp_invappr_(invappr, tp, ni + 1);
        lmmp_copy(dst, invappr + 1, ni);
    }
    TEMP_FREE;
}

mp_limb_t lmmp_div_mulinv_(mp_ptr dstq,
                           mp_ptr numa,
                           mp_size_t na,
                           mp_srcptr numb,
                           mp_size_t nb,
                           mp_srcptr invappr,
                           mp_size_t ni) {
    lmmp_debug_assert(na >= nb && nb >= ni);
    lmmp_debug_assert(ni > 0);
    lmmp_debug_assert(numb[nb - 1] >= 0x8000000000000000ull);
    mp_size_t nq = na - nb, ntp = MIN(ni, nq) + nb;
    mp_limb_t qh;
    TEMP_DECL;
    mp_ptr tp = TALLOC_TYPE(ntp, mp_limb_t);

    numa += nq;
    dstq += nq;

    qh = lmmp_cmp_(numa, numb, nb) >= 0;
    if (qh) {
        lmmp_sub_n_(numa, numa, numb, nb);
	}
    while (nq) {
        if (nq < ni) {
            invappr += ni - nq;
            ni = nq;
        }
        numa -= ni;
        dstq -= ni;
        nq -= ni;

        lmmp_mul_n_(tp, numa + nb, invappr, ni);
        lmmp_assert(lmmp_add_n_(dstq, tp + ni, numa + nb, ni) == 0);

        mp_size_t mn, wn;
        mp_limb_t cy;

        if (nb < DIV_MULINV_MODM_THRESHOLD || (mn = lmmp_fft_next_size_(nb + 1)) >= nb + ni) {
            lmmp_mul_(tp, numb, nb, dstq, ni);  // nb+ni limbs, high 'ni' cancels
		} else {
            // 0<wn<ni<=nb<mn<nb+ni
            wn = nb + ni - mn;

            // x=b*q
            // tp=x mod 2^mn-1
            lmmp_mul_mersenne_(tp, mn, numb, nb, dstq, ni);

            // tp-=ah:0 mod B^mn-1, if result=0, represent it as B^mn-1
            cy = lmmp_sub_nc_(tp, tp, numa + mn, wn, 1);
            if (cy)
                cy = lmmp_sub_1_(tp + wn, tp + wn, mn - wn, 1);
            if (!cy)
                lmmp_inc(tp);

            // if al<<tp,
            if (lmmp_cmp_(numa + nb, tp + nb, mn - nb) < 0) {
                // maybe ah=xh+1 and al<<xl,
                //  so we subtracted 1 too much when tp-=ah,
                //  now tp=xl-1 mod B^mn-1, and 0<=al<<xl-1<B^mn-1, so tp=xl-1
                // or ah=xh and al>=xl,
                //  tp=xl mod B^mn-1, the only possibility is we represented xl=0 as tp=B^mn-1
                // whatever, just inc and then tp=xl
                tp[mn] = 0;  // set a limit
                lmmp_inc(tp);
            }
        }

        mp_limb_t r = numa[nb] - tp[nb];
        cy = lmmp_sub_n_(numa, numa, tp, nb);

        while ((r -= cy) || lmmp_cmp_(numa, numb, nb) >= 0) {
            lmmp_inc(dstq);
            cy = lmmp_sub_n_(numa, numa, numb, nb);
        }
    }
    TEMP_FREE;
    return qh;
}
