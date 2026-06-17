/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../../../include/lammp/impl/tmp_alloc.h"
#include "../../../include/lammp/impl/lglg.h"
#include "../../../include/lammp/impl/longlong.h"
#include "../../../include/lammp/impl/mparam.h"
#include "../../../include/lammp/lmmpn.h"
#include "../../../include/lammp/numth.h"


static inline ulong pow_gt(ulong x, uchar n) {
    ulong ret = 1;
    mp_limb_t hi;
    for (uchar i = 0; i < n; i++) {
        _umul64to128_(ret, x, &ret, &hi);
    }
    if (hi != 0)
        return MP_ULONG_MAX;
    else
        return ret;
}

typedef struct {
    ulong upper;
    ulong lower;
} nthroot_1_bound_t;

static inline void nthroot_1_bound(ulong x, uchar n, nthroot_1_bound_t* bound) {
    // x = 1.xxxx * 2^63
    // frac = x - 2^63
    ulong frac = x - (1ULL << 63);

    uint idx = frac >> (63 - 9);

    ulong log2x_q9_lower = idx == 0 ? 0 : log2_fix32_q9[idx - 1];
    ulong log2x_q9_upper = log2_fix32_q9[idx];

    ulong log2x, L, guess;
    uint F, lo, hi;

    log2x = ((ulong)63 << 32) + log2x_q9_lower;
    log2x /= n;

    L = log2x >> 32;
    F = (uint)(log2x & 0xFFFFFFFF);  // (Q32)

    if (F >= log2_fix32_q9[0]) {
        lo = 0, hi = 511;
        while (lo < hi) {
            uint mid = (lo + hi + 1) >> 1;
            if ((uint)log2_fix32_q9[mid] <= F)
                lo = mid;
            else
                hi = mid - 1;
        }
        // 2^f >= 1 + (lo+1)/512
        guess = 512 + lo + 1;
    } else {
        guess = 512;
    }
    bound->lower = (guess << L) / 512;

    log2x = ((ulong)63 << 32) + log2x_q9_upper;
    log2x /= n;

    L = log2x >> 32;
    F = (uint)(log2x & 0xFFFFFFFF);  // (Q32)

    lo = 0, hi = 511;
    while (lo < hi) {
        uint mid = (lo + hi + 1) >> 1;
        if ((uint)log2_fix32_q9[mid] <= F)
            lo = mid;
        else
            hi = mid - 1;
    }
    lo++;
    // 2^f < 1 + (lo+1)/512
    guess = 512 + lo + 1;
    bound->upper = (guess << L) / 512;
}

ulong lmmp_nthroot_ulong_(ulong x, uchar n) {
    lmmp_param_assert(n > 1);
    lmmp_param_assert(x >= LIMB_B_2);
    nthroot_1_bound_t bound;
    nthroot_1_bound(x, n, &bound);
    ulong lo = bound.lower, hi = bound.upper;
    while (lo + 1 < hi) {
        ulong mid = lo + (hi - lo) / 2;
        ulong p = pow_gt(mid, n);
        if (p <= x) {
            lo = mid;
        } else {
            hi = mid;
        }
    }
    return lo;
}
