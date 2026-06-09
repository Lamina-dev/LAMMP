/*
 * [LAMMP]
 * Copyright (C) [2025-2026] [HJimmyK(Jericho Knox)]
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

#include <stdbool.h>

#include "lmmp.h"

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


/**
 * @brief 计算 a 在2^32下的逆元
 * @param a 待求逆元
 * @warning a%2==1
 * @return 逆元
 */
LAMMP_API uint lmmp_binvert_uint_(uint a);

/**
 * @brief 计算 a 在2^64下的逆元
 * @param a 待求逆元
 * @warning a%2==1
 * @return 逆元
 */
LAMMP_API ulong lmmp_binvert_ulong_(ulong a);

/**
 * @brief 计算 [numa,2] 在B^2下的逆元
 * @param numa 待求逆元指针（长度为 2 个limb）
 * @param dst 结果指针（长度为 2 个limb）
 * @warning numa!=NULL, dst!=NULL, numa[0]%2==1, eqsep(dst,numa)
 */
LAMMP_API void lmmp_binvert_2_(mp_ptr dst, mp_srcptr numa);

/**
 * @brief 计算 [numa,3] 在B^3下的逆元
 * @param numa 待求逆元指针（长度为 3 个limb）
 * @param dst 结果指针（长度为 3 个limb）
 * @warning numa!=NULL, dst!=NULL, numa[0]%2==1, sep(dst,numa)
 */
LAMMP_API void lmmp_binvert_3_(mp_ptr dst, mp_srcptr numa);

/**
 * @brief 计算 [numa,4] 在B^4下的逆元
 * @param numa 待求逆元指针（长度为 4 个limb）
 * @param dst 结果指针（长度为 4 个limb）
 * @warning numa!=NULL, dst!=NULL, numa[0]%2==1, sep(dst,numa)
 */
LAMMP_API void lmmp_binvert_4_(mp_ptr dst, mp_srcptr numa);

/**
 * @brief 计算 [numa,n] 在B^n下的逆元
 * @param numa 待求逆元指针（长度为 n 个limb）
 * @param dst 结果指针（长度为 n 个limb）
 * @param n 结果的 limb 长度
 * @param tp 临时工作区指针（长度为 5*(n+1)/2 个limb）
 * @warning numa!=NULL, dst!=NULL, numa[0]%2==1, sep(dst,numa,tp)
 */
LAMMP_API void lmmp_binvert_n_dc_(mp_ptr dst, mp_srcptr numa, mp_size_t n, mp_ptr tp);

/**
 * @brief 计算 [numa,na] 在B^n 下的逆元
 * @param dst 结果指针（长度为 n 个limb）
 * @param numa 待求逆元指针（长度为 na 个limb）
 * @param na 待求逆元的 limb 长度
 * @param n 结果的 limb 长度
 * @warning n>=na>0, numa!=NULL, dst!=NULL, numa[0]%2==1, sep(dst,numa)
 * @return dst 的实际 limb 长度
 */
// LAMMP_API mp_size_t lmmp_binvert_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_size_t n);

/**
 * @brief 计算两个无符号整数的最大公约数
 * @param u 第一个无符号整数
 * @param v 第二个无符号整数
 * @return 最大公约数
 * @warning u!=0, v!=0
 */
LAMMP_API mp_limb_t lmmp_gcd_11_(mp_limb_t u, mp_limb_t v);

/**
 * @brief 计算两个无符号整数的最大公约数
 * @param up 第一个无符号整数指针
 * @param un 第一个无符号整数的 limb 长度
 * @param v 第二个无符号整数
 * @warning v!=0, up!=NULL, un>0
 * @return 最大公约数
 */
LAMMP_API mp_limb_t lmmp_gcd_1_(mp_srcptr up, mp_size_t un, mp_limb_t vlimb);

/**
 * @brief 计算两个无符号整数的最大公约数
 * @param up 第一个无符号整数指针，长度为 2
 * @param vp 第二个无符号整数指针，长度为 2
 * @param dst 结果指针（长度为 2，两个 limb 都会进行写入，即使最高位可能为0）
 * @warning up!=NULL, vp!=NULL, [up,2]!=0, [vp,2]!=0, dst!=NULL, eqsep(dst,[up|vp])
 * @note 我们不要求 up 和 vp 的高位不为 0，但要求两个数均不可以高低位全为 0
 * @return dst 的实际 limb 长度
 */
LAMMP_API mp_size_t lmmp_gcd_22_(mp_ptr dst, mp_srcptr up, mp_srcptr vp);

/**
 * @brief 计算两个无符号整数的最大公约数
 * @param up 第一个无符号整数指针
 * @param un 第一个无符号整数的 limb 长度
 * @param vp 第二个无符号整数指针，长度为 2
 * @param dst 结果指针（长度至少为 2，两个 limb 都会进行写入，即使最高位可能为0）
 * @warning up!=NULL, un>2, vp!=NULL, vp[1]!=0, dst!=NULL, eqsep(dst,[up|vp])
 * @return dst 的实际 limb 长度
 */
LAMMP_API mp_size_t lmmp_gcd_2_(mp_ptr dst, mp_srcptr up, mp_size_t un, mp_srcptr vp);

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
LAMMP_API mp_size_t lmmp_gcd_basecase_(mp_ptr dst, mp_srcptr up, mp_size_t un, mp_srcptr vp, mp_size_t vn);

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
LAMMP_API mp_size_t lmmp_gcd_lehmer_(mp_ptr dst, mp_srcptr up, mp_size_t un, mp_srcptr vp, mp_size_t vn);

/**
 * @brief 计算两个无符号整数的乘积，对mod取模，商放入 q 中
 * @param a 第一个无符号整数
 * @param b 第二个无符号整数
 * @param q 商的结果指针
 * @param mod 取模数
 * @warning a < mod, b < mod, q!=NULL
 * @return 余数
 */
LAMMP_API ulong lmmp_mulmod_ulong_(ulong a, ulong b, ulong mod, ulongp q);

/**
 * @brief 计算 base^exp 对 mod 取模
 * @param base 底数
 * @param exp 指数
 * @param mod 模数
 * @warning base < mod, mod > 1
 * @return base^exp 对 mod 取模的结果
 */
LAMMP_API uint lmmp_powmod_uint_(uint base, ulong exp, uint mod);

/**
 * @brief 计算 base^exp 对 mod 取模
 * @param base 底数
 * @param exp 指数
 * @param mod 模数
 * @warning base < mod, mod > 1
 * @return base^exp 对 mod 取模的结果
 */
LAMMP_API ulong lmmp_powmod_ulong_(ulong base, ulong exp, ulong mod);

/**
 * @brief 大于n的下一个素数
 * @param n 起始点（不含）
 * @warning 如果 n 大于等于ulong可表示最大的质数，则返回ulong_max
 * @return 大于n的下一个素数
 */
LAMMP_API ulong lmmp_next_prime_ulong_(ulong n);

/**
 * @brief 小于等于n的上一个素数
 * @param n 起始点（含）
 * @warning 如果 n 小于2，则返回 0
 * @return 小于等于n的上一个素数，如果n恰好为素数，则返回 n
 */
LAMMP_API ulong lmmp_prev_prime_ulong_(ulong n);

/**
 * @brief 判断素数
 * @param n 待判断的数
 * @return 若 n 为素数，返回 true，否则返回 false
 */
LAMMP_API bool lmmp_is_prime_uint_(uint n);

/**
 * @brief 判断素数
 * @param n 待判断的数
 * @note 如果 n 的实际值小于2^32，此函数不会调用 lmmp_is_prime_uint_，
 *       如果你可以保证 n 的实际值小于2^32，使用 lmmp_is_prime_uint_ 将会更快
 * @return 若 n 为素数，返回 true，否则返回 false
 */
LAMMP_API bool lmmp_is_prime_ulong_(ulong n);

/**
 * @brief 判断素数（无试除法）
 * @param n 待判断的数（建议为极有可能为素数的数）
 * @note 不进行试除法过滤，适用于判断已被小素数试除法过滤的数或强伪素数
 * @warning n>=2
 * @return 若 n 为素数，返回 true，否则返回 false
 */
LAMMP_API bool lmmp_is_prime_notrial_(ulong n);

/**
 * @brief 计算幂次方需要的limb缓冲区长度 [base,n] ^ exp
 * @param base 底数指针
 * @param n 底数 limb 长度
 * @param exp 指数
 * @warning n>0, base[n-1]!=0, [base,n]>1
 * @return 返回值为 [base,n]^exp 需要的 limb 缓冲区长度（比实际长度多）
 */
LAMMP_API mp_size_t lmmp_pow_size_(mp_srcptr base, mp_size_t n, ulong exp);

/**
 * @brief 计算幂次方需要的limb缓冲区长度 base ^ exp
 * @param base 底数
 * @param exp 指数
 * @warning exp>0, base>=1
 * @return 返回值为 base^exp 需要的 limb 缓冲区长度（比实际长度多）
 */
LAMMP_API mp_size_t lmmp_pow_1_size_(mp_limb_t base, ulong exp);

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
LAMMP_API mp_size_t lmmp_pow_basecase_(mp_ptr dst, mp_size_t rn, mp_srcptr base, mp_size_t n, ulong exp);

/**
 * @brief 计算幂次方 [dst,rn] = [base,1] ^ exp
 * @param dst 结果指针
 * @param rn 结果 limb 长度
 * @param base 底数
 * @param exp 指数
 * @warning 1<=base<=0xf, exp>0
 * @return 返回 dst 的实际 limb 长度
 */
LAMMP_API mp_size_t lmmp_u4_pow_1_(mp_ptr dst, mp_size_t rn, ulong base, ulong exp);

/**
 * @brief 计算幂次方 [dst,rn] = [base,1] ^ exp
 * @param dst 结果指针
 * @param rn 结果 limb 长度
 * @param base 底数
 * @param exp 指数
 * @warning 0<base<=0xff, exp>0
 * @return 返回 dst 的实际 limb 长度
 */
LAMMP_API mp_size_t lmmp_u8_pow_1_(mp_ptr dst, mp_size_t rn, ulong base, ulong exp);

/**
 * @brief 计算幂次方 [dst,rn] = [base,1] ^ exp
 * @param dst 结果指针
 * @param rn 结果 limb 长度
 * @param base 底数
 * @param exp 指数
 * @warning 0<base<=0xffff, exp>0
 * @return 返回 dst 的实际 limb 长度
 */
LAMMP_API mp_size_t lmmp_u16_pow_1_(mp_ptr dst, mp_size_t rn, ulong base, ulong exp);

/**
 * @brief 计算幂次方 [dst,rn] = [base,1] ^ exp
 * @param dst 结果指针
 * @param rn 结果 limb 长度
 * @param base 底数
 * @param exp 指数
 * @warning 0<base<=2^32-1, exp>0
 * @return 返回 dst 的实际 limb 长度
 */
LAMMP_API mp_size_t lmmp_u32_pow_1_(mp_ptr dst, mp_size_t rn, ulong base, ulong exp);

/**
 * @brief 计算幂次方 [dst,rn] = [base,1] ^ exp
 * @param dst 结果指针
 * @param rn 结果 limb 长度
 * @param base 底数
 * @param exp 指数
 * @warning 2^32<=base<=2^64-1, exp>0
 * @return 返回 dst 的实际 limb 长度
 */
LAMMP_API mp_size_t lmmp_u64_pow_1_(mp_ptr dst, mp_size_t rn, ulong base, ulong exp);

/**
 * @brief 计算幂次方 [dst,rn] = [base,1] ^ exp
 * @param dst 结果指针
 * @param base 底数
 * @param exp 指数
 * @warning base>=1, exp>0
 * @return 返回 dst 的实际 limb 长度
 */
LAMMP_API mp_size_t lmmp_pow_1_(mp_ptr dst, mp_size_t rn, mp_limb_t base, ulong exp);

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
LAMMP_API mp_size_t lmmp_pow_win2_(mp_ptr dst, mp_size_t rn, mp_srcptr base, mp_size_t n, ulong exp);

/**
 * @brief 计算大整数幂 [dst,rn] = [base,n] ^ exp
 * @param dst 结果指针
 * @param rn 结果 limb 长度
 * @param base 底数
 * @param n 底数的 limb 长度
 * @param exp 指数
 * @warning n>0, base[n-1]!=0, sep(dst,base), exp>0
 * @return 返回 dst 的实际 limb 长度
 */
LAMMP_API mp_size_t lmmp_pow_(mp_ptr dst, mp_size_t rn, mp_srcptr base, mp_size_t n, ulong exp);

/**
 * @brief 计算 nPr 排列数的 limb 缓冲区长度
 * @param n 排列数的总数
 * @param r 排列数的选择数
 * @param bits 被修改为 nPr 的2的因子数
 * @warning r<=n, bits!=NULL
 * @return nPr 排列数的 limb 缓冲区长度（比实际长度多）
 */
LAMMP_API mp_size_t lmmp_nPr_size_(ulong n, ulong r, mp_bitcnt_t* bits);

/**
 * @brief 计算 nPr 排列数的奇数部分
 * @param dst 结果指针
 * @param rn 结果指针的 limb 长度（nPr_size_ 函数的返回值 - bits/LIMB_BITS）
 * @param n 排列数的总数
 * @param r 排列数的选择数
 * @warning 0xffff>=n>=r, dst!=NULL, rn>0
 * @return 返回 dst 的实际 limb 长度
 */
LAMMP_API mp_size_t lmmp_odd_nPr_ushort_(mp_ptr dst, mp_size_t rn, ulong n, ulong r);

/**
 * @brief 计算 nPr 排列数的奇数部分
 * @param dst 结果指针
 * @param rn 结果指针的 limb 长度（nPr_size_ 函数的返回值 - bits/LIMB_BITS）
 * @param n 排列数的总数
 * @param r 排列数的选择数
 * @warning 0xffffffff>=n>=r, dst!=NULL, rn>0
 * @return 返回 dst 的实际 limb 长度
 */
LAMMP_API mp_size_t lmmp_odd_nPr_uint_(mp_ptr dst, mp_size_t rn, ulong n, ulong r);

/**
 * @brief 计算 nPr 排列数的奇数部分
 * @param dst 结果指针
 * @param rn 结果指针的 limb 长度（nPr_size_ 函数的返回值 - bits/LIMB_BITS）
 * @param n 排列数的总数
 * @param r 排列数的选择数
 * @warning n>=r, dst!=NULL, rn>0
 * @return 返回 dst 的实际 limb 长度
 */
LAMMP_API mp_size_t lmmp_odd_nPr_ulong_(mp_ptr dst, mp_size_t rn, ulong n, ulong r);

/**
 * @brief 计算 nPr 排列数 ( nPr = n! / (n-r)! )
 * @param dst 结果指针
 * @param bits nPr 的2的因子数
 * @param rn 结果指针的 limb 长度
 * @param n 排列数的总数
 * @param r 排列数的选择数
 * @warning n>=r, dst!=NULL, rn>0
 * @return 返回 dst 的实际 limb 长度
 */
LAMMP_API mp_size_t lmmp_nPr_(mp_ptr dst, mp_bitcnt_t bits, mp_size_t rn, ulong n, ulong r);

/**
 * @brief 计算 n! 阶乘的 limb 缓冲区长度
 * @param n 阶乘的阶数
 * @param bits 被修改为 n! 的2的因子数
 * @warning bits!=NULL
 * @return n! 阶乘的 limb 缓冲区长度（比实际长度多）
 */
LAMMP_API mp_size_t lmmp_factorial_size_(uint n, mp_bitcnt_t* bits);

/**
 * @brief 计算 n! 阶乘的奇数部分
 * @param dst 结果指针
 * @param rn 结果指针的 limb 长度（factorial_size_ 函数的返回值 - bits/LIMB_BITS）
 * @param n 阶乘的阶数
 * @warning n>0xffff, dst!=NULL, rn>0
 * @return 返回 dst 的实际 limb 长度
 */
LAMMP_API mp_size_t lmmp_odd_factorial_uint_(mp_ptr dst, mp_size_t rn, uint n);

/**
 * @brief 计算 n! 阶乘
 * @param dst 结果指针
 * @param bits n! 的2的因子数
 * @param rn 结果指针的 limb 长度
 * @param n 阶乘的阶数
 * @warning dst!=NULL, rn>0
 * @return 返回 dst 的实际 limb 长度
 */
LAMMP_API mp_size_t lmmp_factorial_(mp_ptr dst, mp_bitcnt_t bits, mp_size_t rn, uint n);

/**
 * @brief 计算 n!! 双阶乘的 limb 缓冲区长度
 * @param n 双阶乘的阶数
 * @param bits 被修改为 n!! 的2的因子数
 * @warning bits!=NULL
 * @return n!! 双阶乘的 limb 缓冲区长度
 */
LAMMP_API mp_size_t lmmp_2factorial_size_(uint n, mp_bitcnt_t* bits);

/**
 * @brief 计算 n!! 双阶乘
 * @param dst 结果指针
 * @param bits n!! 的2的因子数
 * @param rn 结果指针的 limb 长度
 * @param n 双阶乘的阶数
 * @warning dst!=NULL, rn>0
 * @note 0的双阶乘为1，n为偶数时，n!!=2*4*...*n，n为奇数时，n!!=1*3*...*n
 * @return 返回 dst 的实际 limb 长度
 */
LAMMP_API mp_size_t lmmp_2factorial_(mp_ptr dst, mp_bitcnt_t bits, mp_size_t rn, uint n);

/**
 * @brief 计算hyper阶乘的 limb 缓冲区长度
 * @param n hyper阶乘的阶数
 * @param bits 被修改为 hyper阶乘的2的因子数
 * @warning bits!=NULL
 * @return hyper阶乘的 limb 缓冲区长度
 */
LAMMP_API mp_size_t lmmp_hyperfac_size_(ushort n, mp_bitcnt_t* bits);

/**
 * @brief 计算hyper阶乘（k^k累乘至n）
 * @param dst 结果指针
 * @param bits hyper阶乘的2的因子数
 * @param rn 结果指针的 limb 长度
 * @param n hyper阶乘的阶数
 * @warning dst!=NULL, rn>0
 * @return dst 的实际 limb 长度
 */
LAMMP_API mp_size_t lmmp_hyperfac_(mp_ptr dst, mp_bitcnt_t bits, mp_size_t rn, ushort n);

/**
 * @brief 计算super阶乘的 limb 缓冲区长度
 * @param n super阶乘的阶数
 * @param bits 被修改为 super阶乘的2的因子数
 * @warning bits!=NULL
 * @return hyper阶乘的 limb 缓冲区长度
 */
LAMMP_API mp_size_t lmmp_superfac_size_(ushort n, mp_bitcnt_t* bits);

/**
 * @brief 计算super阶乘（k!累乘至n）
 * @param dst 结果指针
 * @param bits super阶乘的2的因子数
 * @param rn 结果指针的 limb 长度
 * @param n super阶乘的阶数
 * @warning dst!=NULL, rn>0
 * @return dst 的实际 limb 长度
 */
LAMMP_API mp_size_t lmmp_superfac_(mp_ptr dst, mp_bitcnt_t bits, mp_size_t rn, ushort n);

/**
 * @brief 计算质数阶乘的 limb 缓冲区长度
 * @param n 质数阶乘的阶数
 * @return 质数阶乘的 limb 缓冲区长度
 */
LAMMP_API mp_size_t lmmp_primefac_size_(uint n);

/**
* @brief 计算质数阶乘（不超过n的指数累乘）
* @param dst 结果指针
* @param rn 结果指针的 limb 长度
* @param n 质数阶乘的阶数
* @warning dst!=NULL, rn>0
* @return dst 的实际 limb 长度
*/
LAMMP_API mp_size_t lmmp_primefac_(mp_ptr dst, mp_size_t rn, uint n);

/**
 * @brief 计算 nCr 组合数的 limb 缓冲区长度
 * @param n 组合数的总数
 * @param r 组合数的选择数
 * @param bits 被修改为 nCr 的2的因子数
 * @warning r<=n/2, bits!=NULL
 * @return nCr 组合数的 limb 缓冲区长度（比实际长度多 1-2 个 limb）
 */
LAMMP_API mp_size_t lmmp_nCr_size_(uint n, uint r, mp_bitcnt_t* bits);

/**
 * @brief 计算 nCr 组合数的奇数部分
 * @param dst 结果指针
 * @param rn 结果指针的 limb 长度
 * @param n 组合数的总数
 * @param r 组合数的选择数
 * @return 返回 dst 的实际 limb 长度
 * @warning r<=n/2, n<=0xffff, dst!=NULL, rn>0
 */
LAMMP_API mp_size_t lmmp_odd_nCr_ushort_(mp_ptr dst, mp_size_t rn, uint n, uint r);

/**
 * @brief 计算 nCr 组合数的奇数部分
 * @param dst 结果指针
 * @param rn 结果指针的 limb 长度
 * @param n 组合数的总数
 * @param r 组合数的选择数
 * @return 返回 dst 的实际 limb 长度
 * @warning r<=n/2, 0xffff<n, dst!=NULL, rn>0
 */
LAMMP_API mp_size_t lmmp_odd_nCr_uint_(mp_ptr dst, mp_size_t rn, uint n, uint r);

/**
 * @brief 计算 nCr 组合数 ( nCr = n! / (r!(n-r)!) )
 * @param dst 结果指针
 * @param bits nCr 的2的因子数
 * @param rn 结果指针的 limb 长度
 * @param n 组合数的总数
 * @param r 组合数的选择数
 * @return 返回 dst 的实际 limb 长度
 * @warning r<=n/2, n<=0xffffffff, dst!=NULL, rn>0
 */
LAMMP_API mp_size_t lmmp_nCr_(mp_ptr dst, mp_bitcnt_t bits, mp_size_t rn, uint n, uint r);

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
LAMMP_API mp_size_t lmmp_multinomial_size_(const uintp r, uint m, ulong* n);

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
LAMMP_API mp_size_t lmmp_multinomial_(mp_ptr dst, mp_size_t rn, uint n, const uintp r, uint m);

/**
 * @brief 计算等差数列乘积 x(x+m)...(x+n*m)的 limb 缓冲区长度
 * @param x 首项
 * @param n 等差数列共n+1项
 * @param m 公差
 * @warning x>0, m>1, n>0, x+n*m<=0xffffffff
 * @return 等差数列乘积的 limb 缓冲区长度（比实际长度多 1-2 个 limb）
 */
LAMMP_API mp_size_t lmmp_arith_seqprod_size_(uint x, uint n, uint m);

/**
 * @brief 计算等差数列乘积 x(x+m)...(x+n*m)
 * @param dst 结果指针
 * @param rn 结果指针的 limb 长度
 * @param x 首项
 * @param n 等差数列共n+1项
 * @param m 公差（大于1）
 * @warning x>0, m>1, n>0, dst!=NULL, rn>0, x+n*m<=0xffffffff
 * @return 结果的实际的 limb 缓冲区长度
 */
LAMMP_API mp_size_t lmmp_arith_seqprod_(mp_ptr dst, mp_size_t rn, uint x, uint n, uint m);

/**
 * @brief 试除法
 * @param num 被除数
 * @param nn 被除数的 limb 长度
 * @param N 试除法尝试的质数最大值
 * @param rn 结果指针的 limb 长度
 * @warning num!=NULL, nn>0, N>2, rn!=NULL
 * @note 试除法尝试从 2-N 中所有质数进行试除，如果能整除则会插入到返回结果数组中，没有整除的则会返回 NULL。
 *       结果指针请使用 lmmp_free() 函数进行释放。
 * @return 结果指针，返回不超过N，且能整除[np,nn]的素数（从小到大排列），若没有能够整除的素数，则返回NULL
 */
LAMMP_API ushortp lmmp_trialdiv_(mp_srcptr np, mp_size_t nn, ushort N, ushort* rn);

/**
 * @brief 除去[np,nn]中的[dp,dn]的因子
 * @param np 被除数指针，将会被修改为除去因子后的数
 * @param nn 被除数指针的 limb 长度，将会被修改除去因子后的长度
 * @param dp 除数指针
 * @param dn 除数指针的 limb 长度
 * @warning np!=NULL, nn>0, dp!=NULL, dn>0
 * @note 如果[np,nn]能被[dp,dn]整除，则[np,nn]将被修改为除去因子后的数，nn将被修改为除去因子后的长度。
 *       如果不能被整除，则[np,nn]保持不变，并返回0。
 * @return [np,nn]中被[dp,dn]除去的因子的个数，如果不能被整除，则返回0
 */
LAMMP_API mp_size_t lmmp_remove_(mp_ptr np, mp_size_t* nn, mp_srcptr dp, mp_size_t dn);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // LAMMP_NUMTH_H