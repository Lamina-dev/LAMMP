#ifndef __LAMMP_SIGNED_H__
#define __LAMMP_SIGNED_H__

#include "../lmmpn.h"

#ifndef INLINE_
#define INLINE_ static inline
#endif

/**
 * @brief 计算带符号数的加法
 * @param dst 结果指针，自行保证有足够的空间，需要的最多的空间为max(abs(na),abs(nb)) + 1
 * @param numa 第一个数的指针
 * @param na 第一个数的长度，为负数表示此数为负数，绝对值表示实际长度
 * @param numb 第二个数的指针
 * @param nb 第二个数的长度，为负数表示此数为负数，绝对值表示实际长度
 * @return 结果的长度，为负数表示此数为负数，绝对值表示实际长度
 * @warning dst!=NULL, eqsep(dst,[numa|numb])
 */
INLINE_ mp_ssize_t lmmp_add_signed_(mp_ptr dst, mp_srcptr numa, mp_ssize_t na, mp_srcptr numb, mp_ssize_t nb) {
    if (na == 0 && nb != 0) {
        lmmp_copy(dst, numb, LMMP_ABS(nb));
        return nb;
    } else if (na != 0 && nb == 0) {
        lmmp_copy(dst, numa, LMMP_ABS(na));
        return na;
    } else if (na == 0 && nb == 0) {
        dst[0] = 0;
        return 0;
    } else if (na < 0 && nb < 0) {
        na = -na;
        nb = -nb;
        if (na > nb) {
            dst[na] = lmmp_add_(dst, numa, na, numb, nb);
            return dst[na] == 0 ? -na : -(na + 1);
        } else {
            dst[nb] = lmmp_add_(dst, numb, nb, numa, na);
            return dst[nb] == 0 ? -nb : -(nb + 1);
        }
    } else if (na > 0 && nb > 0) {
        if (na > nb) {
            dst[na] = lmmp_add_(dst, numa, na, numb, nb);
            return dst[na] == 0 ? na : na + 1;
        } else {
            mp_limb_t c = lmmp_add_(dst, numb, nb, numa, na);
            dst[nb] = c;
            return dst[nb] == 0 ? nb : nb + 1;
        }
    } else if (na < 0 && nb > 0) {
        na = -na;
        if (na < nb) {
            lmmp_sub_(dst, numb, nb, numa, na);
            while (dst[nb - 1] == 0 && nb > 0) {
                --nb;
            }
            return nb;
        } else if (na > nb) {
            lmmp_sub_(dst, numa, na, numb, nb);
            while (dst[na - 1] == 0 && na > 0) {
                --na;
            }
            return -na;
        } else {
            int cmp = lmmp_cmp_(numa, numb, na);
            if (cmp < 0) {
                lmmp_sub_(dst, numb, nb, numa, na);
                while (dst[nb - 1] == 0 && nb > 0) {
                    --nb;
                }
                return nb;
            } else if (cmp > 0) {
                lmmp_sub_(dst, numa, na, numb, nb);
                while (dst[na - 1] == 0 && na > 0) {
                    --na;
                }
                return -na;
            } else {
                dst[0] = 0;
                return 0;
            }
        }
    } else {
        /* na > 0 && nb < 0 */
        nb = -nb;
        if (na < nb) {
            lmmp_sub_(dst, numb, nb, numa, na);
            while (dst[nb - 1] == 0 && nb > 0) {
                --nb;
            }
            return -nb;
        } else if (na > nb) {
            lmmp_sub_(dst, numa, na, numb, nb);
            while (dst[na - 1] == 0 && na > 0) {
                --na;
            }
            return na;
        } else {
            int cmp = lmmp_cmp_(numa, numb, na);
            if (cmp < 0) {
                lmmp_sub_(dst, numb, nb, numa, na);
                while (dst[nb - 1] == 0 && nb > 0) {
                    --nb;
                }
                return -nb;
            } else if (cmp > 0) {
                lmmp_sub_(dst, numa, na, numb, nb);
                while (dst[na - 1] == 0 && na > 0) {
                    --na;
                }
                return na;
            } else {
                dst[0] = 0;
                return 0;
            }
        }
    }
}

/**
 * @brief 计算带符号数的乘法
 * @param dst 结果指针，自行保证有足够的空间，需要空间为abs(na)+abs(nb)
 * @param numa 第一个数的指针
 * @param na 第一个数的长度，为负数表示此数为负数，绝对值表示实际长度
 * @param numb 第二个数的指针
 * @param nb 第二个数的长度，为负数表示此数为负数，绝对值表示实际长度
 * @return 结果的长度，为负数表示此数为负数，绝对值表示实际长度
 * @warning dst!=NULL, sep(dst,[numa|numb])
 */
INLINE_ mp_ssize_t lmmp_mul_signed_(mp_ptr dst, mp_srcptr numa, mp_ssize_t na, mp_srcptr numb, mp_ssize_t nb) {
    mp_ssize_t sign = ((na > 0) ^ (nb > 0)) ? -1 : 1;
    na = LMMP_ABS(na);
    nb = LMMP_ABS(nb);
    if (na == 0 || nb == 0) {
        dst[0] = 0;
        return 0;
    }
    if (na < nb)
        lmmp_mul_(dst, numb, nb, numa, na);
    else
        lmmp_mul_(dst, numa, na, numb, nb);
    na += nb;
    na -= (dst[na - 1] == 0);
    return sign * na;
}

#ifdef INLINE_
#undef INLINE_
#endif

#endif /* __LAMMP_SIGNED_H__ */