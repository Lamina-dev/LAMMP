#include "../../../include/lammp/numth.h"

typedef struct {
    slong m11, m12;
    slong m21, m22;
} mp_gcd_lehmer_t;

#define LEHMER_MIN_V 0x100000000ull

// hgcd blog: https://www.cnblogs.com/whx1003/p/16217087.html

static void lmmp_gcd_lehmer_step_(mp_limb_t u, mp_limb_t v, mp_gcd_lehmer_t* gcd) {
    lmmp_debug_assert(u >= v);
#define A (gcd->m11)
#define B (gcd->m12)
#define C (gcd->m21)
#define D (gcd->m22)

    if (u < v) {
        LMMP_SWAP(u, v, mp_limb_t);
        A = 0; B = 1;
        C = 1; D = 0;
    } else {
        A = 1; B = 0;
        C = 0; D = 1;
    }

    while (v != 0) {
        lmmp_debug_assert(u >= v);
        slong q = u / v;
        slong t = u % v;

        u = v;
        v = t;
        
        t = A - q * C;
        A = C;
        C = t;
        t = B - q * D;
        B = D;
        D = t;

        if (v < (slong)LEHMER_MIN_V) break;
    }

    return;
#undef A
#undef B
#undef C
#undef D
}

static void lmmp_lehmer_extract_(mp_srcptr up, mp_size_t un, mp_srcptr vp, mp_size_t vn, mp_limb_t* a, mp_limb_t* b) {
    lmmp_debug_assert(un > 0 && vn > 0);
    lmmp_debug_assert(un >= vn);
    lmmp_debug_assert(up != NULL && vp != NULL);
    lmmp_debug_assert(a != NULL && b != NULL);
    // u > v
    if (un == 1) {
        *a = up[0];
        *b = vp[0];
    } else {
        int kz = lmmp_limb_bits_(up[un - 1]);
        *a = up[un - 1] << (64 - kz);
        *a |= up[un - 2] >> kz;
        if (un > vn + 1) {
            *b = 0;
        } else if (un == vn + 1) {
            *b = vp[vn - 1] >> kz;
        } else {
            *b = vp[vn - 1] << (64 - kz);
            *b |= vp[vn - 2] >> kz;
        }
    }
    return;
}

mp_size_t lmmp_gcd_lehmer_(mp_ptr dst, mp_srcptr up, mp_size_t un, mp_srcptr vp, mp_size_t vn) {
    lmmp_debug_assert(un > 0 && vn > 0);
    lmmp_debug_assert(up != NULL && vp != NULL);
    lmmp_debug_assert(dst != NULL);

    if (un < vn) {
        LMMP_SWAP(up, vp, mp_srcptr);
        LMMP_SWAP(un, vn, mp_size_t);
    } else if (un == vn) {
        int cmp = lmmp_cmp_(up, vp, un);
        if (cmp == 0) {
            lmmp_copy(dst, up, un);
            return un;
        } else if (cmp < 0) {
            LMMP_SWAP(up, vp, mp_srcptr);
        }
    }
    // u > v

    mp_gcd_lehmer_t M;
    mp_limb_t x = 0, y = 0;

#define an un
#define bn vn
    TEMP_DECL;
    mp_ptr a = TALLOC_TYPE(an, mp_limb_t);
    mp_ptr b = TALLOC_TYPE(bn, mp_limb_t);
    lmmp_copy(a, up, an);
    lmmp_copy(b, vp, bn);

    // while (1) {
        
    // }



    return 0;
}