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

#include <math.h>
#include <stdbool.h>

#include "lmmp.h"


// 幂运算中，底数长度为 1 的幂运算指数阈值，低于此阈值使用连乘法
#define POW_1_EXP_THRESHOLD 10

// 幂运算中，指数大于此值可能使用win2算法
#define POW_WIN2_EXP_THRESHOLD 50

// 幂运算中，底数长度大于此值可能使用win2算法
#define POW_WIN2_N_THRESHOLD 400

// 排列数，二项式、多项式系数计算中，大于此阈值的质数幂将使用快速幂算法
#define PERMUTATION_PRIME_POW_THRESHOLD 32

// 排列数，二项式、多项式系数计算中，朴素连乘的乘法空间长度阈值
#define PERMUTATION_MUL_MAX_THRESHOLD 20

// 排列数计算中，结果长度小于此阈值的将使用朴素连乘
#define PERMUTATION_RN_BASECASE_THRESHOLD 200

// 排列数计算中，结果长度小于此阈值的将使用哈夫曼队列连乘
#define PERMUTATION_RN_MUL_THRESHOLD 20000

// 排列数计算中，结果长度大于此阈值的将使用质因数分解算法
#define BINOMIAL_RN_BASECASE_THRESHOLD 30

#define LOG2_ 0.693147180559945

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

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
 * @brief 计算两个无符号整数的最大公约数
 * @param u 第一个无符号整数
 * @param v 第二个无符号整数
 * @return 最大公约数
 * @warning u!=0, v!=0
 */
mp_limb_t lmmp_gcd_11_(mp_limb_t u, mp_limb_t v);

/**
 * @brief 计算两个无符号整数的最大公约数
 * @param up 第一个无符号整数指针
 * @param un 第一个无符号整数的 limb 长度
 * @param v 第二个无符号整数
 * @warning v!=0, up!=NULL, un>0
 * @return 最大公约数
 */
mp_limb_t lmmp_gcd_1_(mp_srcptr up, mp_size_t un, mp_limb_t vlimb);

/**
 * @brief 计算两个无符号整数的最大公约数
 * @param up 第一个无符号整数指针，长度为 2
 * @param vp 第二个无符号整数指针，长度为 2
 * @param dst 结果指针（长度为 2，两个 limb 都会进行写入，即使最高位可能为0）
 * @warning up!=NULL, vp!=NULL, [up,2]!=0, [vp,2]!=0, dst!=NULL, eqsep(dst,[up|vp])
 * @note 我们不要求 up 和 vp 的高位不为 0，但要求两个数均不可以高低位全为 0
 * @return dst 的实际 limb 长度
 */
mp_size_t lmmp_gcd_22_(mp_ptr dst, mp_srcptr up, mp_srcptr vp);

/**
 * @brief 计算两个无符号整数的最大公约数
 * @param up 第一个无符号整数指针
 * @param un 第一个无符号整数的 limb 长度
 * @param vp 第二个无符号整数指针，长度为 2
 * @param dst 结果指针（长度至少为 2，两个 limb 都会进行写入，即使最高位可能为0）
 * @warning up!=NULL, un>2, vp!=NULL, vp[1]!=0, dst!=NULL, eqsep(dst,[up|vp])
 * @return dst 的实际 limb 长度
 */
mp_size_t lmmp_gcd_2_(mp_ptr dst, mp_srcptr up, mp_size_t un, mp_srcptr vp);

/**
 * @brief 计算两个无符号整数的最大公约数（不建议使用此算法，更高版本可能被彻底弃用）
 * @param dst 结果指针（长度至少为 min(un,vn)）
 * @param up 第一个无符号整数指针
 * @param un 第一个无符号整数的 limb 长度
 * @param vp 第二个无符号整数指针
 * @param vn 第二个无符号整数的 limb 长度
 * @warning up!=NULL, un>0, vp!=NULL, vn>0, eqsep(dst,[up|vp]), dst!=NULL
 * @note 朴素的辗转相除法，与Lehmer算法具有相似的渐进时间复杂度，但Lehmer算法绝大多数场合更加优秀
 * @return dst 的实际 limb 长度
 */
mp_size_t lmmp_gcd_basecase_(mp_ptr dst, mp_srcptr up, mp_size_t un, mp_srcptr vp, mp_size_t vn);

/**
 * @brief 计算两个无符号整数的最大公约数（Lehmer算法）
 * @param dst 结果指针（长度至少为 min(un,vn)）
 * @param up 第一个无符号整数指针
 * @param un 第一个无符号整数的 limb 长度
 * @param vp 第二个无符号整数指针
 * @param vn 第二个无符号整数的 limb 长度
 * @warning up!=NULL, un>0, vp!=NULL, vn>0, eqsep(dst,[up|vp]), dst!=NULL
 * @return dst 的实际 limb 长度
 */
mp_size_t lmmp_gcd_lehmer_(mp_ptr dst, mp_srcptr up, mp_size_t un, mp_srcptr vp, mp_size_t vn);

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
 * @brief 计算幂次方需要的limb缓冲区长度 [base,n] ^ exp
 * @param base 底数指针
 * @param n 底数 limb 长度
 * @param exp 指数
 * @warning n>0, base[n-1]!=0, [base,n]>1
 * @return 返回值为 [base,n]^exp 需要的 limb 缓冲区长度（比实际长度多 1-2 个limb）
 */
INLINE_ mp_size_t lmmp_pow_size_(mp_srcptr base, mp_size_t n, ulong exp) {
    mp_size_t rn = exp * (n - 1) * LIMB_BITS + ceil((double)exp * log2(base[n - 1]));
    return (rn + LIMB_BITS - 1) / LIMB_BITS + 2; /* more two limbs */
}

/**
 * @brief 计算幂次方需要的limb缓冲区长度 base ^ exp
 * @param base 底数
 * @param exp 指数
 * @warning n>0, base>1
 * @return 返回值为 base^exp 需要的 limb 缓冲区长度（比实际长度多 1-2 个limb）
 */
INLINE_ mp_size_t lmmp_pow_1_size_(mp_limb_t base, ulong exp) {
    return (ceil((double)(exp)*log2((double)base)) + LIMB_BITS - 1) / LIMB_BITS + 2; /* more two limbs */
}

/**
 * @brief 计算奇数次幂算法 [dst,rn] = [base,n] ^ exp
 * @param dst 结果指针
 * @param rn dst 的 limb 缓冲区长度
 * @param base 底数指针
 * @param n 底数的 limb 长度
 * @param exp 指数
 * @warning n>0, base[n-1]!=0, sep(dst,base), [base,n]>1, exp>=3, exp%2==1
 * @return 返回 dst 的实际 limb 长度
 */
mp_size_t lmmp_pow_basecase_(mp_ptr dst, mp_size_t rn, mp_srcptr base, mp_size_t n, ulong exp);

/**
 * @brief 计算幂次方 [dst,rn] = [base,1] ^ exp
 * @param dst 结果指针
 * @param rn 结果 limb 长度
 * @param base 底数
 * @param exp 指数
 * @warning base<=0xf, exp>0
 * @return 返回 dst 的实际 limb 长度
 */
mp_size_t lmmp_1_pow_1_(mp_ptr dst, mp_size_t rn, ulong base, ulong exp);

/**
 * @brief 计算幂次方 [dst,rn] = [base,1] ^ exp
 * @param dst 结果指针
 * @param rn 结果 limb 长度
 * @param base 底数
 * @param exp 指数
 * @warning base<=0xff, exp>0
 * @return 返回 dst 的实际 limb 长度
 */
mp_size_t lmmp_2_pow_1_(mp_ptr dst, mp_size_t rn, ulong base, ulong exp);

/**
 * @brief 计算幂次方 [dst,rn] = [base,1] ^ exp
 * @param dst 结果指针
 * @param rn 结果 limb 长度
 * @param base 底数
 * @param exp 指数
 * @warning base<=0xffff, exp>0
 * @return 返回 dst 的实际 limb 长度
 */
mp_size_t lmmp_4_pow_1_(mp_ptr dst, mp_size_t rn, ulong base, ulong exp);

/**
 * @brief 计算幂次方 [dst,rn] = [base,1] ^ exp
 * @param dst 结果指针
 * @param rn 结果 limb 长度
 * @param base 底数
 * @param exp 指数
 * @warning base<=2^32-1, exp>0
 * @return 返回 dst 的实际 limb 长度
 */
mp_size_t lmmp_8_pow_1_(mp_ptr dst, mp_size_t rn, ulong base, ulong exp);

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
 * @warning base>1, exp>0
 * @note dst缓冲区实际写入的空间为返回值+[0|1]，（必定小于 rn，当然前提是你调用的是 lmmp_pow_1_size_ 函数）
 *       dst缓冲区会被安全的写入，而无需担心 dst 的初始值产生影响，以免调用者额外将缓冲区置零。
 * @return 返回 dst 的实际 limb 长度
 */
INLINE_ mp_size_t lmmp_pow_1_(mp_ptr dst, mp_size_t rn, mp_limb_t base, ulong exp) {
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
}

/**
 * @brief 计算幂次方2比特窗口快速幂算法 [dst,rn] = [base,n] ^ exp
 * @param dst 结果指针
 * @param rn 结果 limb 长度
 * @param base 底数
 * @param n 底数的 limb 长度
 * @param exp 指数
 * @warning n>0, base[n-1]!=0, sep(dst,base), exp>0
 * @return 返回 dst 的实际 limb 长度
 */
mp_size_t lmmp_pow_win2_(mp_ptr dst, mp_size_t rn, mp_srcptr base, mp_size_t n, ulong exp);

/**
 * @brief 计算大整数幂 [dst,rn] = [base,n] ^ exp
 * @param dst 结果指针
 * @param rn 结果 limb 长度
 * @param base 底数
 * @param n 底数的 limb 长度
 * @param exp 指数
 * @warning n>0, base[n-1]!=0, sep(dst,base), exp>0
 * @note dst缓冲区实际写入的空间为返回值+[0|1]，（必定小于 rn，当然前提是你调用的是 lmmp_pow_size_ 函数）
 *       dst缓冲区会被安全的写入，而无需担心 dst 的初始值产生影响，以免调用者额外将缓冲区置零。
 * @return 返回 dst 的实际 limb 长度
 */
mp_size_t lmmp_pow_(mp_ptr dst, mp_size_t rn, mp_srcptr base, mp_size_t n, ulong exp);

/**
 * @brief 判断素数
 * @param n 待判断的数
 * @return 若 n 为素数，返回 true，否则返回 false
 */
bool lmmp_is_prime_ulong_(ulong n);

/**
 * @brief 计算 nPr 排列数的 limb 缓冲区长度
 * @param n 排列数的总数
 * @param r 排列数的选择数
 * @return nPr 排列数的 limb 缓冲区长度（比实际长度多 1-2 个 limb）
 */
INLINE_ mp_size_t lmmp_nPr_size_(ulong n, ulong r) {
    double ln_perm = lgamma(n + 1.0) - lgamma(n - r + 1.0);
    double log2_perm = ln_perm / LOG2_;
    mp_size_t rn = ceil(log2_perm / LIMB_BITS) + 2; /* more two limbs */
    return rn;
}

/**
 * @brief 计算 nPr 排列数 ( nPr = n! / (n-r)! )
 * @param dst 结果指针
 * @param rn 结果指针的 limb 长度
 * @param n 排列数的总数
 * @param r 排列数的选择数
 * @warning 0xffff >= n >= r
 * @return 返回 dst 的实际 limb 长度
 */
mp_size_t lmmp_nPr_short_(mp_ptr dst, mp_size_t rn, ulong n, ulong r);

/**
 * @brief 计算 nPr 排列数 ( nPr = n! / (n-r)! )
 * @param dst 结果指针
 * @param rn 结果指针的 limb 长度
 * @param n 排列数的总数
 * @param r 排列数的选择数
 * @warning 0xffffffff >= n >= r
 * @return 返回 dst 的实际 limb 长度
 */
mp_size_t lmmp_nPr_int_(mp_ptr dst, mp_size_t rn, ulong n, ulong r);

/**
 * @brief 计算 nPr 排列数 ( nPr = n! / (n-r)! )
 * @param dst 结果指针
 * @param rn 结果指针的 limb 长度
 * @param n 排列数的总数
 * @param r 排列数的选择数
 * @warning n >= r
 * @return 返回 dst 的实际 limb 长度
 */
mp_size_t lmmp_nPr_long_(mp_ptr dst, mp_size_t rn, ulong n, ulong r);

/**
 * @brief 计算 nPr 排列数 ( nPr = n! / (n-r)! )
 * @param dst 结果指针
 * @param rn 结果指针的 limb 长度
 * @param n 排列数的总数
 * @param r 排列数的选择数
 * @warning n >= r
 * @note dst缓冲区实际写入的空间为返回值+[0|1]，（必定小于 rn，当然前提是你调用的是 lmmp_nPr_size_ 函数）
 *       dst缓冲区会被安全的写入，而无需担心 dst 的初始值产生影响，以免调用者额外将缓冲区置零。
 * @return 返回 dst 的实际 limb 长度
 */
INLINE_ mp_size_t lmmp_nPr_(mp_ptr dst, mp_size_t rn, ulong n, ulong r) {
    lmmp_debug_assert(n >= r);
    if (n <= 0xffff) 
        return lmmp_nPr_short_(dst, rn, n, r);
    else if (n <= 0xffffffff)
        return lmmp_nPr_int_(dst, rn, n, r);
    else
        return lmmp_nPr_long_(dst, rn, n, r);
}

/**
 * @brief 计算 n! 阶乘的 limb 缓冲区长度
 * @param n 阶乘的阶数
 * @return n! 阶乘的 limb 缓冲区长度（比实际长度多 1-2 个 limb）
 */
INLINE_ mp_size_t lmmp_factorial_size_(uint n) {
    double ln_fact = lgamma(n + 1.0);       
    double log2_fact = ln_fact / LOG2_;
    mp_size_t rn = ceil(log2_fact / LIMB_BITS) + 2; /* more two limbs */
    return rn;
}

/**
 * @brief 计算 n! 阶乘
 * @param dst 结果指针
 * @param rn 结果指针的 limb 长度
 * @param n 阶乘的阶数
 * @return 返回 dst 的实际 limb 长度
 */
INLINE_ mp_size_t lmmp_factorial_(mp_ptr dst, mp_size_t rn, uint n) {
    if (n <= 0xffff) 
        return lmmp_nPr_short_(dst, rn, n, n);
    else
        return lmmp_nPr_int_(dst, rn, n, n);
}

/**
 * @brief 计算 nCr 组合数的 limb 缓冲区长度
 * @param n 组合数的总数
 * @param r 组合数的选择数
 * @return nCr 组合数的 limb 缓冲区长度（比实际长度多 1-2 个 limb）
 */
INLINE_ mp_size_t lmmp_nCr_size_(uint n, uint r) {
    double ln_comb = lgamma(n + 1.0) - lgamma(r + 1.0) - lgamma(n - r + 1.0);
    double log2_comb = ln_comb / LOG2_;
    mp_size_t rn = ceil(log2_comb / LIMB_BITS) + 2; /* more two limbs */
    return rn;
}

/**
 * @brief 计算 nCr 组合数 ( nCr = n! / (r!(n-r)!) )
 * @param dst 结果指针
 * @param rn 结果指针的 limb 长度
 * @param n 组合数的总数
 * @param r 组合数的选择数
 * @return 返回 dst 的实际 limb 长度
 * @warning r<=n/2, n<=0xffff
 */
mp_size_t lmmp_nCr_short_(mp_ptr dst, mp_size_t rn, uint n, uint r);

/**
 * @brief 计算 nCr 组合数 ( nCr = n! / (r!(n-r)!) )
 * @param dst 结果指针
 * @param rn 结果指针的 limb 长度
 * @param n 组合数的总数
 * @param r 组合数的选择数
 * @return 返回 dst 的实际 limb 长度
 * @warning r<=n/2
 */
mp_size_t lmmp_nCr_int_(mp_ptr dst, mp_size_t rn, uint n, uint r);

/**
 * @brief 计算 nCr 组合数 ( nCr = n! / (r!(n-r)!) )
 * @param dst 结果指针
 * @param rn 结果指针的 limb 长度
 * @param n 组合数的总数
 * @param r 组合数的选择数
 * @return 返回 dst 的实际 limb 长度
 * @warning r <= floor(n/2)
 */
INLINE_ mp_size_t lmmp_nCr_(mp_ptr dst, mp_size_t rn, uint n, uint r) {
    lmmp_debug_assert(r <= (n / 2));
    if (n <= 0xffff) 
        return lmmp_nCr_short_(dst, rn, n, r);
    else
        return lmmp_nCr_int_(dst, rn, n, r);
}

/**
 * @brief 计算多项式系数的 limb 缓冲区长度
 * @param r 需要计算的系数的数组
 * @param m 系数的个数
 * @param n 输出变量，将会被修改为 r[i] 的总和，即r1+r2+...+rm
 * @return 多项式系数的 limb 缓冲区长度（比实际长度多 1-2 个 limb）
 * @note 多项式系数为 ( r1+r2+...+rm )! / ( r1! * r2! * ... * rm!)
 * @warning 我们使用 ulong* n 来同时计算 r[i] 的总和，因为 n 可能超过 0xffffffff。
 *          我们预计算 n，这不仅可以作为后续多项式系数函数的参数传入。
 *          同时也请调用者注意判断 n 是否超过了 0xffffffff
 *          这是 lmmp_multinomial_ 函数的限制。
 */
mp_size_t lmmp_multinomial_size_(const uintp r, uint m, ulong* n);

/**
 * @brief 计算多项式系数
 * @param dst 结果指针
 * @param rn 结果指针的 limb 长度
 * @param n r[i] 的总和
 * @param r 需要计算的系数的数组
 * @param m 系数的个数
 * @warning 0<n<=0xffff, 1<m<=0xffff
 * @note 多项式系数为 ( r1+r2+...+rm )! / ( r1! * r2! * ... * rm!)
 * @return 返回 dst 的实际 limb 长度
 */
mp_size_t lmmp_multinomial_short_(mp_ptr dst, mp_size_t rn, uint n, const uintp r, uint m);

/**
 * @brief 计算多项式系数
 * @param dst 结果指针
 * @param rn 结果指针的 limb 长度
 * @param n r[i] 的总和
 * @param r 需要计算的系数的数组
 * @param m 系数的个数
 * @warning m>1, 0xffff<n<=0xffffffff
 * @note 多项式系数为 ( r1+r2+...+rm )! / ( r1! * r2! * ... * rm!)
 * @return 返回 dst 的实际 limb 长度
 */
mp_size_t lmmp_multinomial_int_(mp_ptr dst, mp_size_t rn, uint n, const uintp r, uint m);

/**
 * @brief 计算多项式系数
 * @param dst 结果指针
 * @param rn 结果指针的 limb 长度
 * @param n r[i] 的总和
 * @param r 需要计算的系数的数组
 * @param m 系数的个数
 * @warning m>1, n>0
 * @note 多项式系数为 ( r1+r2+...+rm )! / ( r1! * r2! * ... * rm!)
 * @return 返回 dst 的实际 limb 长度
 */
INLINE_ mp_size_t lmmp_multinomial_(mp_ptr dst, mp_size_t rn, uint n, const uintp r, uint m) {
    lmmp_debug_assert(m > 1);
    lmmp_debug_assert(n > 0);
    if (n <= 0xffff) 
        return lmmp_multinomial_short_(dst, rn, n, r, m);
    else
        return lmmp_multinomial_int_(dst, rn, n, r, m);
}

#undef LOG2_

#ifdef INLINE_
#undef INLINE_
#endif // INLINE_

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // LAMMP_NUMTH_H