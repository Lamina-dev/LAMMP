#include "../../../include/lammp/numth.h"

mp_size_t lmmp_pow_(mp_ptr dst, mp_size_t rn, mp_srcptr base, mp_size_t n, ulong exp) {
    /**
     * 幂函数的逻辑：
     *        首先需要注意的是，我们实现的并不是普通的快速幂算法，我们的basecase算法事实上是1bit遍历的
     *     也就是win1算法，是从高位向低位的遍历，大量的实验都表明从高位向低位遍历bit可以将最后几步乘
     *     法改为平方运算，从而提高运算速度，这也是win2算法可能的优势所在。而对于低位为0的情况，我们
     *     就进入到win1和win2的选择中了。
     *         实验表明，win2算法在指数mod4=3时拥有的性能优势大于win1算法，所以我们在这里选择win2算法。
     *     对于存在低位零的指数，我们通过一些方法，来尽可能减少拷贝，恰好使得最后一次运算可以直接在dst
     *     上进行。
     *         需要注意的是，win1和win2算法基本没有超过20%的性能差距，最短加法链长度的减少并不足以抵消
     *     乘法改为平方运算的优势，尽可能使得最后几步乘法改为平方运算，可以显著提高运算速度。对于非零的情
     *     况，win2算法可以对mod4==3的情况进行优化，猜测这与将最后一步乘法变得尽可能平衡有关。
     *     
     *     有关的issues：
     *     1. https://github.com/Lamina-dev/LAMMP/issues/51
     *     2. https://github.com/Lamina-dev/LAMMP/issues/50
     *     3. https://github.com/Lamina-dev/LAMMP/issues/49
     */

    lmmp_debug_assert(n > 0);
    lmmp_debug_assert(exp > 0);
    lmmp_debug_assert(base[n - 1] != 0);
    if (exp == 1) {
        lmmp_copy(dst, base, n);
        return n;
    } else if (exp == 2) {
        lmmp_sqr_(dst, base, n);
        rn = n << 1;
        rn -= (dst[rn - 1] == 0);
        return rn;
    } else {
        if (n == 1) {
            if (exp <= POW_1_EXP_THRESHOLD) {
                dst[0] = base[0];
                rn = 1;
                for (mp_size_t i = 1; i < exp; ++i) {
                    dst[rn] = lmmp_mul_1_(dst, dst, rn, base[0]);
                    ++rn;
                    rn -= (dst[rn - 1] == 0);
                }
                return rn;
            } else {
                return lmmp_pow_1_(dst, rn, base[0], exp);
            }
        }
        if (exp > POW_WIN2_EXP_THRESHOLD && n > POW_WIN2_N_THRESHOLD) {
            if ((exp % 4 == 3) || (2 * lmmp_limb_popcnt_(exp) >= (lmmp_limb_bits_(exp)))) {
                return lmmp_pow_win2_(dst, rn, base, n, exp);
            }
        }
        if (exp & 1) {
            return lmmp_pow_basecase_(dst, rn, base, n, exp);
        }

        mp_size_t tz = lmmp_tailing_zeros_(exp);
        TEMP_DECL;
        mp_ptr sq = TALLOC_TYPE((rn + 2) >> 1, mp_limb_t);
        exp >>= tz;

        if (tz & 1) {
            if (exp == 1) {
                lmmp_copy(sq, base, n);
                rn = n;
            } else {
                mp_size_t rn1 = lmmp_pow_size_(base, n, exp);
                rn = lmmp_pow_basecase_(sq, rn1, base, n, exp);
            }
            mp_size_t i = 2;
            for (; i <= tz; i += 2) {
                lmmp_sqr_(dst, sq, rn);
                rn <<= 1;
                rn -= (dst[rn - 1] == 0);
                lmmp_sqr_(sq, dst, rn);
                rn <<= 1;
                rn -= (sq[rn - 1] == 0);
            }
            lmmp_sqr_(dst, sq, rn);
            rn <<= 1;
            rn -= (dst[rn - 1] == 0);
        } else {
            if (exp == 1) {
                lmmp_copy(dst, base, n);
                rn = n;
            } else {
                mp_size_t rn1 = lmmp_pow_size_(base, n, exp);
                rn = lmmp_pow_basecase_(dst, rn1, base, n, exp);
            }
            mp_size_t i = 2;
            for (; i <= tz; i += 2) {
                lmmp_sqr_(sq, dst, rn);
                rn <<= 1;
                rn -= (sq[rn - 1] == 0);
                lmmp_sqr_(dst, sq, rn);
                rn <<= 1;
                rn -= (dst[rn - 1] == 0);
            }
        }

        TEMP_FREE;
        return rn;
    }
}
