#include "../../../include/lammp/lmmpn.h"
#include "../../../include/lammp/impl/u128_u192.h"

mp_limb_t lmmp_div_3_2_(mp_ptr restrict numa, mp_srcptr restrict numb, mp_limb_t inv21) {
    mp_limb_t q;
    mp_limb_t r1, r0;
    mp_limb_t a[3] = {numa[0], numa[1], numa[2]};
    _udiv_qr_3by2(q, r1, r0, a[2], a[1], a[0], numb[1], numb[0], inv21);
    numa[1] = r1;
    numa[0] = r0;
    return q;
}

mp_limb_t lmmp_div_1_(mp_ptr dstq, mp_srcptr numa, mp_size_t na, mp_limb_t x) {
    mp_limb_t ah, al;
    if (na == 1) {
        ah = numa[0];
        if (dstq)
            dstq[0] = ah / x;
        return ah % x;
    }
    if (dstq) {
        /*
          ah    al
           X|XXXtttX|XXXmmmX|XXXnnnX|XXX----|
            |000XXXX|tttXXXX|mmmXXXX|nnnXXXX|
                        t    numa[na]

                 ah    al
           X|XXXtttX|XXXmmmX|XXXnnnX|XXX----|
            |000XXXX|tttXXXX|mmmXXXX|nnnXXXX|
                                t
                             numa[na]
        */
        mp_limb_t t = numa[na - 2], q = 0, r = 0;
        const int shift = lmmp_leading_zeros_(x);
        if (shift > 0) {
            const int rshift = LIMB_BITS - shift;
            ah = numa[na - 1] >> rshift;
            t = numa[na - 2];
            al = (numa[na - 1] << shift) | (t >> rshift);
            x <<= shift;
            const mp_limb_t inv = lmmp_inv_1_(x);
            _udiv_qrnnd_preinv(q, r, ah, al, x, inv);
            dstq[na - 1] = q;
            na -= 2;
            while (na-- > 0) {
                ah = r;
                al = t << shift;
                t = numa[na];
                al |= t >> rshift;
                _udiv_qrnnd_preinv(q, r, ah, al, x, inv);
                dstq[na + 1] = q;
            }
            ah = r;
            al = t << shift;
            _udiv_qrnnd_preinv(q, r, ah, al, x, inv);
            dstq[0] = q;
            return r >> shift;
        } else {
            /*
            ah     al
                |000XXXX|tttXXXX|mmmXXXX|nnnXXXX|
                            t    numa[na]
            */
            ah = 0;
            t = numa[na - 2];
            al = numa[na - 1];
            const mp_limb_t inv = lmmp_inv_1_(x);
            q = al / x;
            r = al % x;
            dstq[na - 1] = q;
            na -= 2;
            while (na-- > 0) {
                ah = r;
                al = t;
                t = numa[na];
                _udiv_qrnnd_preinv(q, r, ah, al, x, inv);
                dstq[na + 1] = q;
            }
            ah = r;
            al = t;
            _udiv_qrnnd_preinv(q, r, ah, al, x, inv);
            dstq[0] = q;
            return r;
        }
    } else {
        mp_limb_t t = numa[na - 2], q = 0, r = 0;
        const int shift = lmmp_leading_zeros_(x);
        if (shift > 0) {
            const int rshift = LIMB_BITS - shift;
            ah = numa[na - 1] >> rshift;
            t = numa[na - 2];
            al = (numa[na - 1] << shift) | (t >> rshift);
            x <<= shift;
            const mp_limb_t inv = lmmp_inv_1_(x);
            _udiv_qrnnd_preinv(q, r, ah, al, x, inv);
            na -= 2;
            while (na-- > 0) {
                ah = r;
                al = t << shift;
                t = numa[na];
                al |= t >> rshift;
                _udiv_qrnnd_preinv(q, r, ah, al, x, inv);
            }
            ah = r;
            al = t << shift;
            _udiv_qrnnd_preinv(q, r, ah, al, x, inv);
            return r >> shift;
        } else {
            ah = 0;
            t = numa[na - 2];
            al = numa[na - 1];
            const mp_limb_t inv = lmmp_inv_1_(x);
            q = al / x;
            r = al % x;
            na -= 2;
            while (na-- > 0) {
                ah = r;
                al = t;
                t = numa[na];
                _udiv_qrnnd_preinv(q, r, ah, al, x, inv);
            }
            ah = r;
            al = t;
            _udiv_qrnnd_preinv(q, r, ah, al, x, inv);
            return r;
        }
    }
}

void lmmp_div_2_(mp_ptr dstq, mp_srcptr numa, mp_size_t na, mp_ptr numb) {

}

mp_limb_t lmmp_div_1_s_(mp_ptr restrict dstq, mp_ptr restrict numa, mp_size_t na, mp_limb_t x) {
    mp_limb_t ah, al;
    if (na == 1) {
        ah = numa[0] / x;
        numa[0] %= x;
        return ah;
    }
    /*
          ah    al
           X|XXXtttX|XXXmmmX|XXXnnnX|XXX----|
            |000XXXX|tttXXXX|mmmXXXX|nnnXXXX|
                        t    numa[na]

                 ah    al
           X|XXXtttX|XXXmmmX|XXXnnnX|XXX----|
            |000XXXX|tttXXXX|mmmXXXX|nnnXXXX|
                                t
                             numa[na]
    */
    mp_limb_t t = numa[na - 2], q = 0, r = 0;
    const int shift = lmmp_leading_zeros_(x);
    if (shift > 0) {
        const int rshift = LIMB_BITS - shift;
        ah = numa[na - 1] >> rshift;
        t = numa[na - 2];
        al = (numa[na - 1] << shift) | (t >> rshift);
        x <<= shift;
        const mp_limb_t inv = lmmp_inv_1_(x);
        _udiv_qrnnd_preinv(q, r, ah, al, x, inv);
        const mp_limb_t qh = q;
        na -= 2;
        while (na-- > 0) {
            ah = r;
            al = t << shift;
            t = numa[na];
            al |= t >> rshift;
            _udiv_qrnnd_preinv(q, r, ah, al, x, inv);
            dstq[na + 1] = q;
        }
        ah = r;
        al = t << shift;
        _udiv_qrnnd_preinv(q, r, ah, al, x, inv);
        dstq[0] = q;
        numa[0] = r >> shift;
        return qh;
    } else {
        /*
        ah     al
            |000XXXX|tttXXXX|mmmXXXX|nnnXXXX|
                        t    numa[na]
        */
        ah = 0;
        t = numa[na - 2];
        al = numa[na - 1];
        const mp_limb_t inv = lmmp_inv_1_(x);
        q = al / x;
        r = al % x;
        const mp_limb_t qh = q;
        na -= 2;
        while (na-- > 0) {
            ah = r;
            al = t;
            t = numa[na];
            _udiv_qrnnd_preinv(q, r, ah, al, x, inv);
            dstq[na + 1] = q;
        }
        ah = r;
        al = t;
        _udiv_qrnnd_preinv(q, r, ah, al, x, inv);
        dstq[0] = q;
        numa[0] = r;
        return qh;
    }
}

mp_limb_t lmmp_div_2_s_(mp_ptr dstq, mp_ptr numa, mp_size_t na, mp_srcptr numb) {
    
}

