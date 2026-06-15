/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../../../include/lammp/impl/inlines.h"
#include "../../../include/lammp/impl/mparam.h"
#include "../../../include/lammp/impl/tmp_alloc.h"
#include "../../../include/lammp/lmmpn.h"
#include "../../../include/lammp/numth.h"
#include "../../../include/lammp/impl/longlong.h"


void lmmp_divexact_1_(mp_ptr dst, mp_srcptr np, mp_size_t nn, mp_limb_t d, mp_limb_t dinv) {
    lmmp_param_assert(d % 2 == 1);
    lmmp_param_assert(dinv * d == 1);
    lmmp_param_assert(nn > 0);
    lmmp_debug_assert(dst != NULL && np != NULL);
    mp_limb_t c = 0;
    mp_limb_t l, s, lo, hi, q;
    mp_size_t i;
    for (i = 0; i < nn - 1; i++) {
        s = np[i];
        l = s - c;
        c = l > s;
        q = l * dinv;
        dst[i] = q;
        _umul64to128_(q, d, &lo, &hi);
        c += hi;
    }
    s = np[i];
    l = s - c;
    c = l > s;
    q = l * dinv;
    dst[i] = q;
    lmmp_debug_assert(c == 0);
}

void lmmp_divexact_2_(mp_ptr dst, mp_srcptr np, mp_size_t nn, mp_srcptr restrict dp, mp_srcptr restrict dinv) {
    lmmp_param_assert(dp[0] % 2 == 1);
    lmmp_param_assert(nn > 1);
    lmmp_debug_assert(dst != NULL && np != NULL);
    mp_limb_t c[2] = {0, 0};
    mp_limb_t l[2], s[2], q[2], t[4];
    mp_size_t i;
    mp_limb_t d0 = dp[0], d1 = dp[1];
    mp_limb_t ddinv0 = dinv[0], ddinv1 = dinv[1];

    if (nn % 2 == 0) {
        for (i = 0; i < nn - 2; i += 2) {
            s[0] = np[i];
            s[1] = np[i + 1];
            _u128sub(l, s, c);
            c[0] = _u128cmp(s, l) ? 1 : 0;
            c[1] = 0;
            _umul128to128_(l[1], l[0], ddinv1, ddinv0, q);
            dst[i] = q[0];
            dst[i + 1] = q[1];
            _umul128to256_(q[1], q[0], d1, d0, t);
            _u128add(c, c, t + 2);
        }
        s[0] = np[i];
        s[1] = np[i + 1];
        _u128sub(l, s, c);
        c[0] = _u128cmp(s, l);
        _umul128to128_(l[1], l[0], ddinv1, ddinv0, q);
        dst[i] = q[0];
        lmmp_debug_assert(c[0] == 0);
    } else {
        i = 0;
        if (nn >= 5) {
            for (; i < nn - 4; i += 2) {
                s[0] = np[i];
                s[1] = np[i + 1];
                _u128sub(l, s, c);
                c[0] = _u128cmp(s, l);
                c[1] = 0;
                _umul128to128_(l[1], l[0], ddinv1, ddinv0, q);
                dst[i] = q[0];
                dst[i + 1] = q[1];
                _umul128to256_(q[1], q[0], d1, d0, t);
                _u128add(c, c, t + 2);
            }
        }
        s[0] = np[i];
        s[1] = np[i + 1];
        _u128sub(l, s, c);
        c[0] = _u128cmp(s, l);
        _umul128to128_(l[1], l[0], ddinv1, ddinv0, q);
        dst[i] = q[0];
        dst[i + 1] = q[1];
        lmmp_debug_assert(c[0] == 0);
    }
}

static inline void lmmp_mullo_n_(
    mp_ptr    restrict  dst,
    mp_srcptr restrict numa,
    mp_srcptr restrict numb,
    mp_size_t             n,
    mp_ptr    restrict   tp
) {
    if (n < MULLO_DC_THRESHOLD) {
        lmmp_mullo_dc_(dst, numa, numb, tp, n);
    } else {
        lmmp_mullo_fft_(dst, numa, numb, n, tp);
    }
}

void lmmp_divexact_unbalanced_(
    mp_ptr              dst,
    mp_srcptr            np,
    mp_size_t            nn,
    mp_srcptr restrict   dp,
    mp_size_t            dn,
    mp_ptr    restrict dinv
) {
    lmmp_param_assert(dst != NULL && np != NULL && dp != NULL);
    lmmp_param_assert(dn > 0);
    lmmp_param_assert(nn >= dn);
    lmmp_param_assert((dp[0] & 1) != 0);

    TEMP_DECL;
    mp_ptr restrict tp = TALLOC_TYPE(4 * dn, mp_limb_t);
    if (dinv == NULL) {
        dinv = TALLOC_TYPE(dn, mp_limb_t);
        lmmp_binvert_n_dc_(dinv, dp, dn, tp);
    }
#define c (tp)     // [tp, dn]
#define l (tp + dn) // [tp + dn, dn]
#define scratch (tp + 2 * dn) // [tp + 2 * dn, 2*dn]
    mp_size_t i = 0, qn = nn - dn + 1;
    mp_limb_t ca;
    lmmp_zero(c, dn);
    if (qn > dn) {
        for (; i < qn - dn; i += dn) {
            lmmp_sub_n_(l, np + i, c, dn);
            ca = lmmp_cmp_(l, np + i, dn) == 1 ? 1 : 0;
            lmmp_mullo_n_(dst + i, l, dinv, dn, scratch);
            lmmp_mul_n_(scratch, dst + i, dp, dn);
            if (ca) {
                lmmp_add_1_(c, scratch + dn, dn, 1);
            } else {
                lmmp_copy(c, scratch + dn, dn);
            }
        }
    }
    lmmp_sub_n_(l, np + i, c, dn);
    lmmp_mullo_n_(dst + i, l, dinv, qn - i, scratch);
    TEMP_FREE;
#undef c
#undef l
#undef scratch
}

void lmmp_divexact_basecase_(mp_ptr dst, mp_ptr np, mp_size_t nn, mp_srcptr restrict dp, mp_size_t dn, mp_limb_t dinv) {
    lmmp_param_assert(dst != NULL && np != NULL && dp != NULL);
    lmmp_param_assert(dn > 0);
    lmmp_param_assert(nn >= dn);
    lmmp_param_assert((dp[0] & 1) != 0);
    lmmp_param_assert((dp[0] * dinv) == 1);
    mp_size_t i;
    mp_limb_t q;
    mp_limb_t hi;
    mp_size_t qn = nn - dn + 1;
    for (i = 0; i < qn; i++) {
        q = dinv * np[i];
        hi = lmmp_submul_1_(np + i, dp, dn, q);
        lmmp_debug_assert(np[i] == 0);
#if LAMMP_DEBUG_ASSERT_CHECK == 1
        if (hi && i + dn < nn) {
            lmmp_dec_1(np + i + dn, hi);
        } else {
            lmmp_debug_assert(hi == 0);
        }
#else
        if (hi) {
            lmmp_dec_1(np + i + dn, hi);
        }
#endif
        dst[i] = q;
    }
}

void lmmp_divexact_divide_(mp_ptr restrict dst, mp_srcptr restrict np, mp_size_t nn, mp_srcptr restrict dp, mp_size_t dn) {
    lmmp_param_assert(dst != NULL && np != NULL && dp != NULL);
    lmmp_param_assert(dn > 0);
    lmmp_param_assert(nn >= dn);
    lmmp_param_assert((dp[0] & 1) != 0);

    mp_size_t qn = nn - dn + 1;
    lmmp_param_assert(qn <= dn);

    TEMP_DECL;
    mp_ptr restrict tp = TALLOC_TYPE((5 * qn + 5) / 2, mp_limb_t);
    mp_ptr restrict dinv = TALLOC_TYPE(qn, mp_limb_t);
    lmmp_binvert_n_dc_(dinv, dp, qn, tp);
    lmmp_mullo_n_(dst, np, dinv, qn, tp);
    TEMP_FREE;
}

void lmmp_divexact_(mp_ptr dst, mp_srcptr np, mp_size_t nn, mp_srcptr restrict dp, mp_size_t dn) {
    lmmp_param_assert(dst != NULL && np != NULL && dp != NULL);
    lmmp_param_assert(dn > 0);
    lmmp_param_assert(nn >= dn);
    lmmp_param_assert((dp[0] & 1) != 0);

    if (dn == 1) {
        mp_limb_t dinv = lmmp_binvert_ulong_(dp[0]);
        lmmp_divexact_1_(dst, np, nn, dp[0], dinv);
    } else if (dn == 2) {
        mp_limb_t dinv[2];
        lmmp_binvert_2_(dinv, dp);
        lmmp_divexact_2_(dst, np, nn, dp, dinv);
    } else {
        mp_size_t qn = nn - dn + 1;
        if (nn < DIVEXACT_NN_THRESHOLD && dn < DIVEXACT_BASECASE_THRESHOLD) {
            mp_limb_t dinv = lmmp_binvert_ulong_(dp[0]);
            TEMP_DECL;
            mp_ptr restrict tp = TALLOC_TYPE(nn, mp_limb_t);
            lmmp_copy(tp, np, nn);
            lmmp_divexact_basecase_(dst, tp, nn, dp, dn, dinv);
            TEMP_FREE;
        } else if (qn < dn) {
            TEMP_DECL;
            mp_ptr restrict tp;
            if (dst == np) {
                tp = TALLOC_TYPE(nn, mp_limb_t);
                lmmp_copy(tp, np, nn);
                np = tp;
            }
            lmmp_divexact_divide_(dst, np, nn, dp, dn);
            TEMP_FREE;
        } else {
            lmmp_divexact_unbalanced_(dst, np, nn, dp, dn, NULL);
        }
    }
}