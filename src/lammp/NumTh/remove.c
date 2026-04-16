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
 * qp 为商，rp 为余数，divp 为被除数，divn 为被除数长度，numb 为除数，nb 为除数长度
 * 如果无法整除，则返回0，否则返回除数 qp 的长度
 */
static inline mp_size_t try_div_(mp_ptr qp, mp_ptr rp, mp_srcptr divp, mp_size_t divn, mp_srcptr numb, mp_size_t nb) {
    if (divn < nb) {
        return 0;
    } else {
        int cmp;
        if (divn > nb)
            cmp = 1;
        else
            cmp = lmmp_cmp_(divp, numb, nb);
        if (cmp == 0) {
            qp[0] = 1;
            return 1;
        } else if (cmp < 0) {
            return 0;
        } else {
            // 拷贝原始数，当无法整除时，再进行恢复
            lmmp_div_(qp, rp, divp, divn, numb, nb);
            if (lmmp_zero_q_(rp, nb)) {
                // 整除
                divn = divn - nb + 1;
                while (divn > 0 && qp[divn - 1] == 0) --divn;
                return divn;
            } else {
                // 无法整除
                return 0;
            }
        }
    }
}

mp_size_t lmmp_remove_(mp_ptr np, mp_size_t* nn, mp_srcptr dp, mp_size_t dn) {
    lmmp_param_assert(np != NULL && *nn > 0);
    lmmp_param_assert(dp != NULL && dn > 0);

    mp_srcptr pd_pow[MAX_EXP];
    mp_size_t pn_pow[MAX_EXP];
    // qp as quotient, rp as remainder, divp as divdend
    mp_ptr qp, rp, divp;
    mp_ptr prod;
    mp_size_t divn = *nn, qn;
    mp_size_t ret = 0;
    int i, j;
    pd_pow[0] = dp;
    pn_pow[0] = dn;

    TEMP_DECL;

    qp = TALLOC_TYPE(*nn, mp_limb_t);
    rp = TALLOC_TYPE(*nn, mp_limb_t);

    divp = np;

    for (i = 1; i < MAX_EXP; i++) {
        // if qn == 0, means cannot be divided by pd_pow[i - 1]
        if (qn = try_div_(qp, rp, divp, divn, pd_pow[i - 1], pn_pow[i - 1])) {
            divn = qn;
            LMMP_SWAP(qp, divp, mp_ptr);

            ret += 1 << (i - 1);
            pn_pow[i] = 2 * pn_pow[i - 1];
            if (divn < pn_pow[i]) {
                ++i;
                break;
            }
            prod = TALLOC_TYPE(pn_pow[i], mp_limb_t);
            lmmp_sqr_(prod, pd_pow[i - 1], pn_pow[i - 1]);
            pn_pow[i] -= (prod[pn_pow[i] - 1] == 0) ? 1 : 0;
            pd_pow[i] = prod;
        } else {
            break;
        }
    }
    for (j = i - 1; j > 0; --j) {
        if (qn = try_div_(qp, rp, divp, divn, pd_pow[j - 1], pn_pow[j - 1])) {
            divn = qn;
            LMMP_SWAP(qp, divp, mp_ptr);

            ret += 1 << (j - 1);
        }
    }
    if (qn == 0) {
        // 无法整除
        lmmp_copy(np, divp, divn);
        *nn = divn;
    } else {
        // 整除
        lmmp_copy(np, qp, qn);
        *nn = qn;
    }

    TEMP_FREE;
    return ret;
}