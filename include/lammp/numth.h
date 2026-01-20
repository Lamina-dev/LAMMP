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

#include <math.h>

typedef uint8_t uchar;
typedef int8_t schar;
typedef uint16_t ushort;
typedef int16_t sshort;
typedef uint32_t uint;
typedef uint64_t ulong;
typedef int32_t sint;
typedef int64_t slong;
typedef uint8_t* ucharp;
typedef int8_t* scharp;
typedef uint16_t* ushortp;
typedef int16_t* sshortp;
typedef uint32_t* uintp;
typedef int32_t* sintp;
typedef uint64_t* ulongp;
typedef int64_t* slongp;

#define ULONG_BITS 64
#define SLONG_BITS 64

#ifndef INLINE_
#define INLINE_ static inline
#endif // INLINE_

/**
 * @brief 计算两个无符号整数的乘积，对mod取模，商放入 q 中
 * @param a 第一个无符号整数
 * @param b 第二个无符号整数
 * @param q 商的结果指针
 * @param mod 取模数
 * @warning a < mod, b < mod, q!=NULL
 * @return 余数
 */
ulong lmmp_mulmod_ulong_(ulong a, ulong b, ulong mod, ulongp q);

/**
 * @brief 计算 base^exp 对 mod 取模
 * @param base 底数
 * @param exp 指数
 * @param mod 模数
 * @warning base < mod
 * @return base^exp 对 mod 取模的结果
 */
ulong lmmp_powmod_ulong_(ulong base, ulong exp, ulong mod);

/**
 * @brief 计算幂次方需要的limb长度 [base,n] ^ exp
 * @param base 底数指针
 * @param n 底数 limb 长度
 * @param exp 指数
 * @warning n>0, base[n-1]!=0, [base,n]>1
 * @return 无返回值，结果储存在dst中
 */
INLINE_ mp_size_t lmmp_pow_size_(mp_srcptr base, mp_size_t n, ulong exp) {
    mp_size_t rn = exp * (n - 1) * LIMB_BITS + ceil((double)exp * log2(base[n - 1]));
    return (rn + LIMB_BITS - 1) / LIMB_BITS + 1; /* more one limb for carry */
}

/**
 * @brief 计算幂次方朴素快速幂算法 [dst,rn] = [base,n] ^ exp
 * @param dst 结果指针
 * @param base 底数指针
 * @param n 底数的 limb 长度
 * @param exp 指数
 * @warning n>0, base[n-1]!=0, sep[dst|base], [base,n]>1, exp>0
 * @return 返回 dst 的实际 limb 长度
 */
mp_size_t lmmp_pow_basecase_(mp_ptr dst, mp_srcptr base, mp_size_t n, ulong exp);

/**
 * @brief 计算幂次方 [dst,rn] = [base,1] ^ exp
 * @param dst 结果指针
 * @param rn 结果 limb 长度
 * @param base 底数
 * @param exp 指数
 * @warning base<=0xf, exp>0
 * @return 返回 dst 的实际 limb 长度
 */
mp_size_t lmmp_1_pow_1_(mp_ptr dst, mp_size_t rn, uchar base, ulong exp);

/**
 * @brief 计算幂次方 [dst,rn] = [base,1] ^ exp
 * @param dst 结果指针
 * @param rn 结果 limb 长度
 * @param base 底数
 * @param exp 指数
 * @warning base<=0xff, exp>0
 * @return 返回 dst 的实际 limb 长度
 */
mp_size_t lmmp_2_pow_1_(mp_ptr dst, mp_size_t rn, uchar base, ulong exp);

/**
 * @brief 计算幂次方 [dst,rn] = [base,1] ^ exp
 * @param dst 结果指针
 * @param rn 结果 limb 长度
 * @param base 底数
 * @param exp 指数
 * @warning base<=0xffff, exp>0
 * @return 返回 dst 的实际 limb 长度
 */
mp_size_t lmmp_4_pow_1_(mp_ptr dst, mp_size_t rn, ushort base, ulong exp);

/**
 * @brief 计算幂次方 [dst,rn] = [base,1] ^ exp
 * @param dst 结果指针
 * @param rn 结果 limb 长度
 * @param base 底数
 * @param exp 指数
 * @warning base<=2^32-1, exp>0
 * @return 返回 dst 的实际 limb 长度
 */
mp_size_t lmmp_8_pow_1_(mp_ptr dst, mp_size_t rn, uint base, ulong exp);

/**
 * @brief 计算幂次方 [dst,rn] = [base,1] ^ exp
 * @param dst 结果指针
 * @param rn 结果 limb 长度
 * @param base 底数
 * @param exp 指数
 * @warning 2^32<=base<=2^64-1, exp>0
 * @return 返回 dst 的实际 limb 长度
 */
mp_size_t lmmp_16_pow_1_(mp_ptr dst, mp_size_t rn, ulong base, ulong exp);

/**
 * @brief 计算幂次方 [dst,rn] = [base,1] ^ exp
 * @param dst 结果指针
 * @param base 底数
 * @param exp 指数
 * @warning base>1, sep[dst|base], exp>0
 * @return 返回 dst 的实际 limb 长度
 */
INLINE_ mp_size_t lmmp_pow_1_(mp_ptr dst, mp_limb_t base, ulong exp) {
    mp_size_t rn = ceil((double)exp * log2(base));
    rn = (rn + LIMB_BITS - 1) / LIMB_BITS + 1; /* more one limb for carry */
    if (base <= (mp_limb_t)0xf) {
        return lmmp_1_pow_1_(dst, rn, base, exp);
    } else if (base <= (mp_limb_t)0xff) {
        return lmmp_2_pow_1_(dst, rn, base, exp);
    } else if (base <= (mp_limb_t)0xffff) {
        return lmmp_4_pow_1_(dst, rn, base, exp);
    } else if (base <= (mp_limb_t)0xffffffff) {
        return lmmp_8_pow_1_(dst, rn, base, exp);
    } else {
        return lmmp_16_pow_1_(dst, rn, base, exp);
    }
    return 0;
}

/**
 * @brief 计算幂次方3比特窗口快速幂算法 [dst,rn] = [base,n] ^ exp
 * @param dst 结果指针
 * @param base 底数
 * @param n 底数的 limb 长度
 * @param exp 指数
 * @warning n>0, base[n-1]!=0, sep[dst|base], exp>0
 * @return 返回 dst 的实际 limb 长度
 */
mp_size_t lmmp_pow_win3_(mp_ptr dst, mp_srcptr base, mp_size_t n, ulong exp);

/**
 * @brief 计算幂次方2比特窗口快速幂算法 [dst,rn] = [base,n] ^ exp
 * @param dst 结果指针
 * @param base 底数
 * @param n 底数的 limb 长度
 * @param exp 指数
 * @warning 0<n, base[n-1]!=0, sep[dst|base], exp>0
 * @return 返回 dst 的实际 limb 长度
 */
mp_size_t lmmp_pow_win2_(mp_ptr dst, mp_srcptr base, mp_size_t n, ulong exp);

typedef struct prime_short {
    ushortp pri;    // prime 数组指针
    ulongp mmp;     // prime 位图指针（1为合数，0为素数）
    ushort prin;    // prime 数量
    ushort mmp_cal; // prime 位图容量
    ushort N;       // 不超过 N 的素数表
} pri_short;

typedef struct prime_int {
    uintp pri;    // prime 数组指针
    ulongp mmp;   // prime 位图指针（1为合数，0为素数）
    uint prin;    // prime 数量
    uint mmp_cal; // prime 位图容量
    uint N;       // 不超过 N 的素数表
} pri_int;

extern uchar lmmp_pri_char_table[];

#define PRI_MMP_ZERO 3 // 位图的初始化值 11000000...

INLINE_ size_t lmmp_prime_size_(ulong n) {
    if (n < 100) {
        return n / 3 + 2;
    } else {
        return ceil(1.05 *(double)n / (log(n) - 1.1));
    }
}

/**
 * @brief 初始化素数表
 * @param p 素数表指针
 * @param n 素数表大小
 */
void lmmp_prime_short_init_(pri_short* p, ushort n);

/**
 * @brief 释放素数表
 * @param p 素数表指针
 */
void lmmp_prime_short_free_(pri_short* p);

/**
 * @brief 初始化素数表
 * @param p 素数表指针
 * @param n 素数表大小
 */
void lmmp_prime_int_init_(pri_int* p, uint n);

/**
 * @brief 释放素数表
 * @param p 素数表指针
 */
void lmmp_prime_int_free_(pri_int* p);

/**
 * @brief 判断素数
 * @param n 待判断的数
 * @return 若 n 为素数，返回 true，否则返回 false
 */
bool lmmp_is_prime_ulong_(ulong n);

#ifdef INLINE_
#undef INLINE_
#endif // INLINE_

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // LAMMP_NUMTH_H