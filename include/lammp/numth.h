/*
 * [LAMMP]
 * Copyright (C) [2025] [HJimmyK/LAMINA]
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef LAMMP_NUMTH_H
#define LAMMP_NUMTH_H

#include "lmmpn.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief 计算幂次方需要的limb长度 [base,n] ^ exp
 * @param base 底数指针
 * @param n 底数 limb 长度
 * @param exp 指数
 * @warning n>0, base[n-1]!=0, [base,n]>1
 * @return 无返回值，结果储存在dst中
 */
mp_size_t lmmp_pow_size_(mp_srcptr base, mp_size_t n, mp_limb_t exp);

/**
 * @brief 计算幂次方 [dst,rn] = [base,n] ^ exp
 * @param dst 结果指针
 * @param base 底数指针
 * @param n 底数 limb 长度
 * @param exp 指数
 * @warning n>0, base[n-1]!=0, sep[dst|base], [base,n]>1
 * @return 返回 dst 的实际 limb 长度
 */
mp_size_t lmmp_pow_basecase_(mp_ptr dst, mp_srcptr base, mp_size_t n, mp_limb_t exp);

/**
 * @brief 计算幂次方 [dst,rn] = [base,1] ^ exp
 * @param dst 结果指针
 * @param rn 结果 limb 长度
 * @param base 底数
 * @param exp 指数
 * @warning 1<base<=0xf
 * @return 返回 dst 的实际 limb 长度
 */
mp_size_t lmmp_1_pow_1_(mp_ptr dst, mp_size_t rn, mp_limb_t base, mp_limb_t exp);

/**
 * @brief 计算幂次方 [dst,rn] = [base,1] ^ exp
 * @param dst 结果指针
 * @param rn 结果 limb 长度
 * @param base 底数
 * @param exp 指数
 * @warning 1<base<=0xff
 * @return 返回 dst 的实际 limb 长度
 */
mp_size_t lmmp_2_pow_1_(mp_ptr dst, mp_size_t rn, mp_limb_t base, mp_limb_t exp);

/**
 * @brief 计算幂次方 [dst,rn] = [base,1] ^ exp
 * @param dst 结果指针
 * @param rn 结果 limb 长度
 * @param base 底数
 * @param exp 指数
 * @warning 1<base<=0xffff
 * @return 返回 dst 的实际 limb 长度
 */
mp_size_t lmmp_4_pow_1_(mp_ptr dst, mp_size_t rn, mp_limb_t base, mp_limb_t exp);

/**
 * @brief 计算幂次方 [dst,rn] = [base,1] ^ exp
 * @param dst 结果指针
 * @param rn 结果 limb 长度
 * @param base 底数
 * @param exp 指数
 * @warning 1<base<=2^32-1, exp = exp % 2^48 (实际计算的指数是 exp % 2^48)
 * @return 返回 dst 的实际 limb 长度
 */
mp_size_t lmmp_8_pow_1_(mp_ptr dst, mp_size_t rn, mp_limb_t base, mp_limb_t exp);

/**
 * @brief 计算幂次方 [dst,rn] = [base,1] ^ exp
 * @param dst 结果指针
 * @param rn 结果 limb 长度
 * @param base 底数
 * @param exp 指数
 * @warning 1<base<=2^64-1, exp = exp % 2^40 (实际计算的指数是 exp % 2^40)
 * @return 返回 dst 的实际 limb 长度
 */
mp_size_t lmmp_16_pow_1_(mp_ptr dst, mp_size_t rn, mp_limb_t base, mp_limb_t exp);

/**
 * @brief 计算幂次方 [dst,rn] = [base,1] ^ exp
 * @param dst 结果指针
 * @param base 底数
 * @param exp 指数
 * @warning base>1, sep[dst|base]
 * @return 返回 dst 的实际 limb 长度
 */
mp_size_t lmmp_pow_1_(mp_ptr dst, mp_limb_t base, mp_limb_t exp);

#undef INLINE_

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // LAMMP_NUMTH_H