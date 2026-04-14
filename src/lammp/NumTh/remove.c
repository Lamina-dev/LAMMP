/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../../../include/lammp/numth.h"
#include "../../../include/lammp/lmmpn.h"
#include "../../../include/lammp/impl/tmp_alloc.h"

/*
事实上，我们可以知道，[np,nn]至多为[dp,dn]的2^64次方
2^64 - 1 - 2 - 4 - 8 - ... - 2^63 最为接近2^64
故，理论上MAX_EXP = 63

但是，实际上，[np,nn]几乎不可能超过[dp,dn]的2^48次方
2^48 - 1 - 2 - 4 - 8 - ... - 2^47 最为接近2^48
故，我们使用MAX_EXP = 48
*/
#define MAX_EXP 48

/**
 * 如果 *mp 不能被整除，则返回false；并无任何操作，如果 *mp 能被整除，则返回true。
 * 且 *tp 被修改为商，然后交换 *mp 和 *tp
 */
static inline bool try_div_(mp_ptr* tp, mp_ptr* mp, mp_size_t* nn, mp_srcptr numb, mp_size_t nb) {
    if (*nn < nb) {
        return false;
    } else {
        int cmp;
        if (*nn > nb)
            cmp = 1;
        else
            cmp = lmmp_cmp_(*mp, numb, nb);
        if (cmp == 0) {
            *mp[0] = 1;
            *nn = 1;
            return true;
        } else if (cmp < 0) {
            return false;
        } else {
            lmmp_div_(*tp, *mp, *mp, *nn, numb, nb);
            if (lmmp_zero_q_(*mp, nb)) {
                // 整除
                *nn = *nn - nb + 1;
                while (*nn > 0 && (*tp)[*nn - 1] == 0) --(*nn);
                LMMP_SWAP(*tp, *mp, mp_ptr);
                return true;
            } else {
                return false;
            }
        }
    }
}

mp_size_t lmmp_remove_(mp_ptr np, mp_size_t* nn, mp_srcptr dp, mp_size_t dn) {
    lmmp_param_assert(np != NULL && *nn > 0);
    lmmp_param_assert(dp != NULL && dn > 0);
    mp_srcptr pd_pow[MAX_EXP + 1];
    mp_size_t pn_pow[MAX_EXP + 1];
    lmmp_zero(pn_pow, MAX_EXP + 1);

    mp_ptr prod;
    mp_size_t n = *nn;
    mp_size_t ret = 0;
    pd_pow[0] = dp;
    pn_pow[0] = dn;

    TEMP_DECL;
    mp_ptr tp = TALLOC_TYPE(n - dn + 1, mp_limb_t);
    mp_ptr mp = np;
    for (int i = 1; i <= MAX_EXP; ++i) {
        if (!try_div_(&tp, &mp, &n, pd_pow[i - 1], pn_pow[i - 1])) break;
        ret += 1 << (i - 1);
        if (n < pn_pow[i - 1] * 2 - 1) break;
        prod = TALLOC_TYPE(pn_pow[i - 1] * 2, mp_limb_t);
        lmmp_sqr_(prod, pd_pow[i - 1], pn_pow[i - 1]);
        pd_pow[i] = prod;
        pn_pow[i] = pn_pow[i - 1] * 2;
        pn_pow[i] -= (pd_pow[i][pn_pow[i] - 1] == 0) ? 1 : 0;
    }
    prod = TALLOC_TYPE(n, mp_limb_t);
    mp_size_t pn;
    for (;;) {
        lmmp_copy(prod, mp, n);
        pn = n;
        if (!try_div_(&tp, &mp, &n, pd_pow[0], pn_pow[0])) {
            // 此时*mp为余数，prod才是原本的已除尽的数
            lmmp_copy(np, prod, pn);
            break;
        }
        ++ret;
        for (int i = 1; i <= MAX_EXP; ++i) {
            if (!try_div_(&tp, &mp, &n, pd_pow[i], pn_pow[i])) break;
            ret += 1 << i;
        }
    }
    *nn = n;
    TEMP_FREE;
    return ret;
}