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

/*
        本库的实现部分灵感来源于或改编自

                GNU-MP (https://gmplib.org/),

        尤其是其中的一些宏定义(MACROS)和汇编(masm)代码。

        注意事项:
        UCASE_: 危险宏，仅在当前头文件中使用，使用后会立即取消定义
        UCASE: 安全宏
        _lcase_: 宏内部使用的临时变量名
                （部分内部多层嵌套宏可能不严格这样，为了避免命名冲突，部分
                  宏混用了 _lcase 命名，也有无下划线的，但外部接口头文件均
                  严格遵守此约定）
        lmmp_lcase_: 危险的函数名/宏函数名（可能存在命名冲突或内存安全风险）
        lmmp_lcase: 安全的函数名/宏函数名（经过命名规范校验）
*/

#ifndef LAMMP_LMMPN_H
#define LAMMP_LMMPN_H

#include <malloc.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "lmmp.h"


// 除法阈值：当操作数规模超过此值时，使用分治除法算法
#define DIV_DIVIDE_THRESHOLD 50
// 乘法逆元L阈值：用于选择乘法逆元计算策略的临界值（L维度）
#define DIV_MULINV_L_THRESHOLD 477
// 乘法逆元N阈值：用于选择乘法逆元计算策略的临界值（N维度）
#define DIV_MULINV_N_THRESHOLD 1736

// 牛顿迭代求逆阈值：超过此规模使用牛顿迭代法求逆
#define INV_NEWTON_THRESHOLD 21
// 模M求逆阈值：超过此规模使用模M优化的求逆算法
#define INV_MODM_THRESHOLD 734

// 模M乘法逆元阈值：用于选择模M乘法逆元计算策略的临界值
#define DIV_MULINV_MODM_THRESHOLD 477

// 牛顿迭代开方阈值：超过此规模使用牛顿迭代法开方
#define SQRT_NEWTON_THRESHOLD 50
// 模M牛顿迭代开方阈值：超过此规模使用模M优化的牛顿迭代开方
#define SQRT_NEWTON_MODM_THRESHOLD 734

// Toom-22乘法阈值：超过此规模使用Toom-22乘法
#define MUL_TOOM22_THRESHOLD 20
// Toom-X2乘法阈值：较短乘数小于此值使用Toom-X2不平衡乘法
#define MUL_TOOMX2_THRESHOLD 30
// Toom-33乘法阈值：超过此规模使用Toom-33乘法
#define MUL_TOOM33_THRESHOLD 65
// FFT乘法阈值：超过此规模使用快速傅里叶变换(FFT)乘法
#define MUL_FFT_THRESHOLD 1736

// 模F FFT乘法阈值：用于模F场景下的FFT乘法策略选择
#define MUL_FFT_MODF_THRESHOLD 477

// 转字符串除法阈值：字符串转换时选择除法算法的临界值
#define TO_STR_DIVIDE_THRESHOLD 20
// 转字符串基数幂阈值：字符串转换时基数幂计算的策略选择临界值
#define TO_STR_BASEPOW_THRESHOLD 30
// 从字符串解析除法阈值：字符串解析时选择除法算法的临界值
#define FROM_STR_DIVIDE_THRESHOLD 45
// 从字符串解析基数幂阈值：字符串解析时基数幂计算的策略选择临界值
#define FROM_STR_BASEPOW_THRESHOLD 100

#define INLINE_ static inline

// L1缓存大小，请将此值设置为实际单核CPU的L1缓存大小（字节数）
// 8192 字节通常远远小于现代CPU的L1缓存大小，但仍然可以满足分块需要了
#define L1_CACHE_SIZE 8192

#define LIMB_BITS 64
#define LIMB_BYTES 8
#define LOG2_LIMB_BITS 6
#define LIMB_MAX (~(mp_limb_t)0)

// L1缓存分块大小
#define PART_SIZE (L1_CACHE_SIZE / LIMB_BYTES / 4)


#ifdef __cplusplus
extern "C" {
#endif

INLINE_ bool lmmp_endian(void) {
    int num = 1;
    return (*(char*)&num) == 0; 
}

/**
 * @brief 计算满足 2^k > x 的最小自然数k
 * @param x 输入的64位无符号整数
 * @return 满足条件的最小自然数k
 */
int lmmp_limb_bits_(mp_limb_t x);
/**
 * @brief 计算一个64位无符号整数中1的个数
 * @param x 输入的64位无符号整数
 * @return 1的个数
 */
int lmmp_limb_popcnt_(mp_limb_t x);

/**
 * @brief 计算一个单精度数(limb)中前导零的个数
 * @param x 输入的64位无符号整数
 * @return 前导零的位数（范围：0~64）
 */
int lmmp_leading_zeros_(mp_limb_t x);

/**
 * @brief 计算一个单精度数(limb)中末尾零的个数
 * @param x 输入的64位无符号整数
 * @return 末尾零的位数（范围：0~64）
 */
int lmmp_tailing_zeros_(mp_limb_t x);

/**
 * @brief 计算两个64位无符号整数相乘的高位结果 (a*b)/2^64
 * @param a 第一个64位无符号整数
 * @param b 第二个64位无符号整数
 * @return 乘积的高64位结果
 */
mp_limb_t lmmp_mulh_(mp_limb_t a, mp_limb_t b);

/**
 * @brief 计算两个64位无符号整数相乘的128位结果 (a*b)
 * @param dst 输出结果缓冲区，存储乘积结果，长度为 2
 * @param a 第一个64位无符号整数
 * @param b 第二个64位无符号整数
 * @return 无返回值
 */
void lmmp_mullh_(mp_limb_t a, mp_limb_t b, mp_ptr dst);

/**
 * @brief Toom插值计算（5点插值），用于Toom-33和Toom-42乘法算法
 * @param dst 输出结果缓冲区，存储插值计算结果
 * @param v2 输入参数：v(2)插值点值
 * @param vm1 输入参数：v(-1)插值点值
 * @param n 操作数的单精度数(limb)长度
 * @param spt 分段大小参数，用于内存分段处理
 * @param vm1_neg 符号标记：v(-1)是否为负数（1表示负，0表示正）
 * @param vinf0 无穷远点插值的低64位值
 */
void lmmp_toom_interp5_(mp_ptr dst, mp_ptr v2, mp_ptr vm1, mp_size_t n, mp_size_t spt, int vm1_neg, mp_limb_t vinf0);

/**
 * @brief 基础平方运算 [dst,2*na] = [numa,na]^2
 * @param dst 输出结果缓冲区，长度至少为2*na
 * @param numa 输入操作数，长度为na
 * @param na 输入操作数的 limb 长度
 * @warning na>0，sep(dst,numa)
 * @return 无返回值，结果存储在dst中
 */
void lmmp_sqr_basecase_(mp_ptr dst, mp_srcptr numa, mp_size_t na);

/**
 * @brief Toom-2平方运算 [dst,2*na] = [numa,na]^2
 * @param dst 输出结果缓冲区，长度至少为 2*na
 * @param numa 输入操作数，长度为na
 * @param na 输入操作数的 limb 长度
 * @warning na>0，sep(dst,numa)，na需大于特定阈值
 * @return 无返回值，结果存储在dst中
 */
void lmmp_sqr_toom2_(mp_ptr dst, mp_srcptr numa, mp_size_t na);

/**
 * @brief Toom-3平方运算 [dst,2*na] = [numa,na]^2
 * @param dst 输出结果缓冲区，长度至少为2*na
 * @param numa 输入操作数，长度为na
 * @param na 输入操作数的单精度数(limb)长度
 * @warning na>0，sep(dst,numa)，na需大于特定阈值
 * @return 无返回值，结果存储在dst中
 */
void lmmp_sqr_toom3_(mp_ptr dst, mp_srcptr numa, mp_size_t na);

/**
 * @brief 基础乘法运算 [dst,na+nb] = [numa,na] * [numb,nb]
 * @param dst 输出结果缓冲区，长度至少为 na+nb
 * @param numa 第一个输入操作数，长度为 na
 * @param na 第一个操作数的 limb 长度
 * @param numb 第二个输入操作数，长度为 nb
 * @param nb 第二个操作数的 limb 长度
 * @warning 0<nb<=na，sep(dst,[numa|numb])
 * @return 无返回值，结果存储在dst中
 */
void lmmp_mul_basecase_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_srcptr numb, mp_size_t nb);

/**
 * @brief Toom-22乘法运算 [dst,na+nb] = [numa,na] * [numb,nb]
 * @param dst 输出结果缓冲区，长度至少为 na+nb
 * @param numa 第一个输入操作数，长度为 na
 * @param na 第一个操作数的 limb 长度
 * @param numb 第二个输入操作数，长度为 nb
 * @param nb 第二个操作数的 limb 长度
 * @warning 4/5<=nb/na<=1，nb>=5，sep(dst,[numa|numb])
 * @return 无返回值，结果存储在dst中
 */
void lmmp_mul_toom22_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_srcptr numb, mp_size_t nb);

/**
 * @brief Toom-32乘法运算 [dst,na+nb] = [numa,na] * [numb,nb]
 * @param dst 输出结果缓冲区，长度至少为 na+nb
 * @param numa 第一个输入操作数，长度为 na
 * @param na 第一个操作数的 limb 长度
 * @param numb 第二个输入操作数，长度为 nb
 * @param nb 第二个操作数的 limb 长度
 * @warning 5/9<=nb/na<=4/5，nb>=12，sep(dst,[numa|numb])
 * @return 无返回值，结果存储在dst中
 */
void lmmp_mul_toom32_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_srcptr numb, mp_size_t nb);

/**
 * @brief Toom-33乘法运算 [dst,na+nb] = [numa,na] * [numb,nb]
 * @param dst 输出结果缓冲区，长度至少为 na+nb
 * @param numa 第一个输入操作数，长度为 na
 * @param na 第一个操作数的 limb 长度
 * @param numb 第二个输入操作数，长度为 nb
 * @param nb 第二个操作数的 limb 长度
 * @warning 4/5<=nb/na<=1，nb>=26，sep(dst,[numa|numb])
 * @return 无返回值，结果存储在dst中
 */
void lmmp_mul_toom33_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_srcptr numb, mp_size_t nb);

/**
 * @brief Toom-42乘法运算 [dst,na+nb] = [numa,na] * [numb,nb]
 * @param dst 输出结果缓冲区，长度至少为 na+nb
 * @param numa 第一个输入操作数，长度为 na
 * @param na 第一个操作数的 limb 长度
 * @param numb 第二个输入操作数，长度为 nb
 * @param nb 第二个操作数的 limb 长度
 * @warning 1/3<=nb/na<=5/9，nb>=20，sep(dst,[numa|numb])
 * @return 无返回值，结果存储在dst中
 */
void lmmp_mul_toom42_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_srcptr numb, mp_size_t nb);

/**
 * @brief 计算满足 >=n 的最小费马/梅森乘法可行尺寸
 * @param n 输入的目标尺寸
 * @return 满足条件的SSA乘法最小尺寸
 */
mp_size_t lmmp_fft_next_size_(mp_size_t n);

/**
 * @brief 费马数模乘法 [dst,rn+1]=[numa,na]*[numb,nb] mod B^rn+1
 * @param dst 输出结果缓冲区，长度至少为 rn+1
 * @param rn 模运算的阶数参数
 * @param numa 第一个输入操作数，长度为 na
 * @param na 第一个操作数的 limb 长度
 * @param numb 第二个输入操作数，长度为 nb
 * @param nb 第二个操作数的 limb 长度
 * @warning 0<=[numa,na]<2*B^rn, 0<=[numb,nb]<2*B^rn, sep(dst,[numa|numb])
 * @return 无返回值，结果存储在dst中
 */
void lmmp_mul_fermat_(mp_ptr dst, mp_size_t rn, mp_srcptr numa, mp_size_t na, mp_srcptr numb, mp_size_t nb);

/**
 * @brief 梅森数模乘法 [dst,rn] = [numa,na]*[numb,nb] mod B^rn-1
 * @param dst 输出结果缓冲区，长度至少为 rn
 * @param rn 模运算的阶数参数
 * @param numa 第一个输入操作数，长度为 na
 * @param na 第一个操作数的 limb 长度
 * @param numb 第二个输入操作数，长度为 nb
 * @param nb 第二个操作数的 limb 长度
 * @warning 0<=[numa,na]<B^rn, 0<=[numb,nb]<B^rn, sep(dst,[numa|numb])
 * @return 无返回值，结果存储在dst中，
 */
void lmmp_mul_mersenne_(mp_ptr dst, mp_size_t rn, mp_srcptr numa, mp_size_t na, mp_srcptr numb, mp_size_t nb);

/**
 * @brief FFT乘法运算 [dst,na+nb] = [numa,na] * [numb,nb]
 * @param dst 输出结果缓冲区，长度至少为 na+nb
 * @param numa 第一个输入操作数，长度为 na
 * @param na 第一个操作数的 limb 长度
 * @param numb 第二个输入操作数，长度为 nb
 * @param nb 第二个操作数的 limb 长度
 * @warning ???<=nb<=na, sep(dst,[numa|numb])
 * @return 无返回值，结果存储在dst中
 */
void lmmp_mul_fft_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_srcptr numb, mp_size_t nb);

/**
 * 大数平方操作 [dst,2*na] = [numa,na]^2
 * @warning na>0, sep(dst,numa)
 * @param dst 平方结果输出指针（需要2*na的limb长度）
 * @param numa 源操作数指针
 * @param na limb长度
 */
INLINE_ void lmmp_sqr_(mp_ptr dst, mp_srcptr numa, mp_size_t na) {
    if (na < MUL_TOOM22_THRESHOLD)
        lmmp_sqr_basecase_(dst, numa, na);
    else if (na < MUL_TOOM33_THRESHOLD)
        lmmp_sqr_toom2_(dst, numa, na);
    else if (na < MUL_FFT_THRESHOLD)
        lmmp_sqr_toom3_(dst, numa, na);
    else
        lmmp_mul_fft_(dst, numa, na, numa, na);
}

/**
 * 等长大数乘法操作 [dst,2*n] = [numa,n] * [numb,n]
 * @warning n>0, sep(dst,[numa|numb])
 *       特殊情况: n==1时dst<=numa+1是允许的
 *                 n==2时dst<=numa是允许的
 * @param dst 乘积结果输出指针（需要 2*n 的 limb 长度）
 * @param numa 第一个乘数指针
 * @param numb 第二个乘数指针
 * @param n limb长度
 */
INLINE_ void lmmp_mul_n_(mp_ptr dst, mp_srcptr numa, mp_srcptr numb, mp_size_t n) {
    if (n < MUL_TOOM22_THRESHOLD)
        lmmp_mul_basecase_(dst, numa, n, numb, n);
    else if (n < MUL_TOOM33_THRESHOLD)
        lmmp_mul_toom22_(dst, numa, n, numb, n);
    else if (n < MUL_FFT_THRESHOLD)
        lmmp_mul_toom33_(dst, numa, n, numb, n);
    else
        lmmp_mul_fft_(dst, numa, n, numb, n);
}

/**
 * @brief 1阶逆元计算 (inv1)
 * @param x 输入的64位无符号整数，最高位为1(MSB(x)=1)
 * @return 计算结果：(B^2-1)/x - B
 * @warning MSB(x)=1, 即x>=2^63
 */
mp_limb_t lmmp_inv_1_(mp_limb_t x);

/**
 * @brief 2-1阶逆元计算 (inv21)
 * @param xh 输入数的高64位部分
 * @param xl 输入数的低64位部分
 * @return 计算结果：(B^3-1)/(xh*B+xl) - B
 * @warning MSB(xh)=1, 即xh>=2^63
 */
mp_limb_t lmmp_inv_2_1_(mp_limb_t xh, mp_limb_t xl);

/**
 * @brief 近似逆元计算（牛顿迭代法）
 * @param dst 输出结果缓冲区，长度为na
 * @param numa 输入操作数，长度为na
 * @param na 输入操作数的 limb 长度
 * @warning na>4, MSB(numa)=1, sep(dst,numa)
 * @return 无返回值，结果存储在dst中，[dst,na]=(B^(2*na)-1)/[numa,na]
 */
void lmmp_inv_basecase_(mp_ptr dst, mp_srcptr numa, mp_size_t na);

/**
 * @brief 近似逆元计算（牛顿迭代法）
 * @param dst 输出结果缓冲区，长度为na
 * @param numa 输入操作数，长度为na
 * @param na 输入操作数的 limb 长度
 * @warning na>4, MSB(numa)=1, sep(dst,numa)
 * @return 无返回值，结果存储在dst中，[dst,na]=(B^(2*na)-1)/[numa,na]+[0|-1]
 */
void lmmp_invappr_newton_(mp_ptr dst, mp_srcptr numa, mp_size_t na);

/**
 * @brief 近似逆元计算 (invappr)
 * @param dst 输出结果缓冲区，长度为na
 * @param numa 输入操作数，长度为na
 * @param na 输入操作数的 limb 长度
 * @warning na>0, MSB(numa)=1, sep(dst,numa)
 * @return 无返回值，结果存储在dst中，[dst,na] = (B^(2*na)-1)/[numa,na] - B^na + [0|-1]
 */
INLINE_ void lmmp_invappr_(mp_ptr dst, mp_srcptr numa, mp_size_t na) {
    if (na < INV_NEWTON_THRESHOLD)
        lmmp_inv_basecase_(dst, numa, na);
    else
        lmmp_invappr_newton_(dst, numa, na);
}

/**
 * @brief 3/2位除法运算 [numa,2]=[numa,3] mod [numb,2]
 * @param numa 输入被除数（长度3），运算后存储余数（长度2）
 * @param numb 输入除数（长度2）
 * @param inv21 除数的2-1阶逆元（提前计算好的inv21([numb,2])）
 * @return 商值（单精度数）
 * @warning [numa,3]<[numb,2]*B, MSB(numb)=1, inv21=inv21([numb,2])
 */
mp_limb_t lmmp_div_3_2_(mp_ptr numa, mp_srcptr numb, mp_limb_t inv21);

/**
 * @brief 单精度数除法
 * @param dstq 输出商的缓冲区（可为NULL，此时仅计算余数）
 * @param numa 输入被除数，长度为na
 * @param na 被除数的 limb 长度
 * @param x 除数（单个 limb ）
 * @return 除法余数（单个 limb ）
 * @warning na>0, x!=0, eqsep(dstq,numa), dstq>=numa-1 是可以接受的
 * @note if (dstq!=NULL) [dstq,na] = [numa,na] div x
 */
mp_limb_t lmmp_div_1_(mp_ptr dstq, mp_srcptr numa, mp_size_t na, mp_limb_t x);

/**
 * @brief 双精度数除法 (除数为2个limb)
 * @param dstq 输出商的缓冲区，长度至少为na-1
 * @param numa 输入被除数（长度na）
 * @param na 被除数的 limb 长度
 * @param numb 输入除数（长度2）[numb,2]=[numa,na] mod [numb,2]
 * @warning na>=2, numb[1]!=0, eqsep(dstq,numa), dstq>=numa 是可以接受的
 * @note if (dstq!=NULL) [dstq,na-1]=[numa,na] div [numb,2]
 */
void lmmp_div_2_(mp_ptr dstq, mp_srcptr numa, mp_size_t na, mp_ptr numb);

/**
 * @brief 基础除法运算
 * @param dstq 输出商的缓冲区，长度至少为na-nb
 * @param numa 输入被除数（长度na），运算后存储余数（长度nb）
 * @param na 被除数的单精度数(limb)长度
 * @param numb 输入除数，长度为nb
 * @param nb 除数的单精度数(limb)长度
 * @param inv21 除数的2-1阶逆元（inv21([numb+nb-2,2])）
 * @return 商的最高位（qh）
 * @warning na>=nb>=3, MSB(numb)=1, inv21=inv21([numb+nb-2,2]), sep(dstq,numa,numb)
 * @note qh:[dstq,na-nb]=[numa,na] div [numb,nb], [numa,na-nb]=[numa,na] mod [numb,nb], return qh
 */
mp_limb_t lmmp_div_basecase_(mp_ptr dstq, mp_ptr numa, mp_size_t na, mp_srcptr numb, mp_size_t nb, mp_limb_t inv21);

/**
 * @brief 分治除法运算
 * @param dstq 输出商的缓冲区，长度至少为na-nb
 * @param numa 输入被除数（长度na），运算后存储余数（长度nb）
 * @param na 被除数的单精度数(limb)长度
 * @param numb 输入除数，长度为nb
 * @param nb 除数的单精度数(limb)长度
 * @param inv21 除数的2-1阶逆元（inv21([numb+nb-2,2])）
 * @return 商的最高位（qh）
 * @warning na>=2*nb, nb>=6, MSB(numb)=1, inv21=inv21([numb+nb-2,2]), sep(dstq,numa,numb)
 * @note qh:[dstq,na-nb]=[numa,na] div [numb,nb], [numa,na-nb]=[numa,na] mod [numb,nb], return qh
 */
mp_limb_t lmmp_div_divide_(mp_ptr dstq, mp_ptr numa, mp_size_t na, mp_srcptr numb, mp_size_t nb, mp_limb_t inv21);

/**
 * @brief 计算预计算逆元的尺寸
 * @param nq 商的 limb 长度
 * @param nb 除数的 limb 长度
 * @return 计算需要预计算逆元尺寸ni（ni<=nb）
 * @note 用于已归一化除法([nq+nb]/[nb]=[nq])的逆元 ni 尺寸
 */
INLINE_ mp_size_t lmmp_div_inv_size_(mp_size_t nq, mp_size_t nb) {
    mp_size_t ni, b;
    if (nq > nb) {
        b = (nq - 1) / nb + 1;  // ceil(nq/nb), number of blocks
        ni = (nq - 1) / b + 1;  // ceil(nq/b)
    } else if (3 * nq > nb) {
        ni = (nq - 1) / 2 + 1;  // b=2
    } else {
        ni = (nq - 1) / 1 + 1;  // b=1
    }
    return ni;
}

/**
 * @brief 除法前的逆元预计算，[dst,ni] = invappr( (ni+1 MSLs of numa) + 1 ) / B
 * @param dst 输出预计算逆元的缓冲区，长度为ni
 * @param numa 输入操作数，长度为na
 * @param na 输入操作数的 limb 长度
 * @param ni 预计算逆元的目标尺寸
 * @warning na>=ni>0, MSB(numa)=1, eqsep(dst,numa)
 * @note if (ni=na) [dst,na] = (B^(2*na)-1) / [numa,na] - B^na
 */
void lmmp_inv_prediv_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_size_t ni);

/**
 * @brief 乘法逆元除法
 * @param dstq 输出商的缓冲区，长度至少为na-nb
 * @param numa 输入被除数（长度na），运算后存储余数（长度nb）
 * @param na 被除数的 limb 长度
 * @param numb 输入除数，长度为nb
 * @param nb 除数的单精度数(limb)长度
 * @param invappr 预计算的近似逆元，长度为ni
 * @param ni 预计算逆元的尺寸
 * @return 商的最高位（qh）
 * @warning na>=nb>=ni>0, MSB(numb)=1, [invappr,ni]=invprediv([numb,nb]), sep(dstq,numa,numb,invappr))
 * @note qh:[dstq,na-1]=[numa,na] div x, [numa,1]=[numa,na] mod x, return qh
 */
mp_limb_t lmmp_div_mulinv_(mp_ptr dstq, mp_ptr numa, mp_size_t na, mp_srcptr numb, mp_size_t nb, mp_srcptr invappr,
                           mp_size_t ni);

/**
 * @brief 单精度数除法（除数为1个limb）
 * @param dstq 输出商的缓冲区，长度至少为na-1
 * @param numa 输入被除数（长度na），运算后存储余数（长度1）
 * @param na 被除数的 limb 长度
 * @param x 除数（单个 limb ）
 * @return 商的最高位（qh）
 * @warning na>1, MSB(x)=1, sqp(dstq,numa)
 * @note qh:[dstq,na-1]=[numa,na] div x, [numa,1]=[numa,na] mod x, return qh
 */
mp_limb_t lmmp_div_1_s_(mp_ptr dstq, mp_ptr numa, mp_size_t na, mp_limb_t x);

/**
 * @brief 双精度数除法（除数为2个limb）
 * @param dstq 输出商的缓冲区，长度至少为na-2
 * @param numa 输入被除数（长度na），运算后存储余数（长度2）
 * @param na 被除数的 limb 长度
 * @param numb 输入除数，长度为2
 * @return 商的最高位（qh）
 * @warning na>2, MSB(numb)=1, sep(dstq,numa,numb)
 * @note qh:[dstq,na-2]=[numa,na] div [numb,2], [numa,2]=[numa,na] mod [numb,2], return qh
 */
mp_limb_t lmmp_div_2_s_(mp_ptr dstq, mp_ptr numa, mp_size_t na, mp_srcptr numb);

/**
 * @brief 除法运算
 * @param dstq 输出商的缓冲区，长度至少为na-nb
 * @param numa 输入被除数（长度na），运算后存储余数（长度nb）
 * @param na 被除数的 limb 长度
 * @param numb 输入除数，长度为nb
 * @param nb 除数的 limb 长度
 * @return 商的最高位（qh）
 * @warning na>=nb>0, MSB(numb)=1, sep(dstq,numa,numb)
 * @note qh:[dstq,na-nb]=[numa,na] div [numb,nb], [numa,nb]=[numa,na] mod [numb,nb], return qh
 */
mp_limb_t lmmp_div_s_(mp_ptr dstq, mp_ptr numa, mp_size_t na, mp_srcptr numb, mp_size_t nb);

typedef struct lmmp_mp_base_t_ {
    // 单个limb能容纳的基数的最大幂次
    // 二的幂次存储 log2(base)
    // large_base = base ^ digits_in_limb
    mp_limb_t large_base;
    // ceiling(2^64*log2(base)/log2(2^64))
    // N 位 base 数最多需要 N * lg_base / 2^64 + 1 个 limb
    mp_limb_t lg_base;
    // N 位二进制数最多需要 N * inv_lg_base / 2^64 + 1 个 base 进制位
    // ceiling(2^64/log2(base))
    mp_limb_t inv_lg_base;
    // 单个limb可容纳的最大基数位数
    // floor(64/log2(base))
    int digits_in_limb;
    // 基数（2~256）
    int base;
} mp_base_t;

extern const mp_base_t lmmp_bases_[257];

typedef struct lmmp_mp_basepow_t_ {
    // 基数幂值(base^digits)
    mp_ptr p;
    // p的 limb 长度
    mp_size_t np;
    // 归一化p的逆元
    mp_ptr invp;
    // invp的有效长度
    mp_size_t ni;
    // 去除的末尾零 limb 长度
    mp_size_t zeros;
    // 基数幂的指数（log_base(p)）
    mp_size_t digits;
    // p归一化时的移位位数
    int norm_cnt;
    // 基数
    int base;
} mp_basepow_t;

// 计算整数的绝对值
#define LMMP_ABS(x) ((x) >= 0 ? (x) : -(x))
// 返回两个数中的较小值
#define LMMP_MIN(l, o) ((l) < (o) ? (l) : (o))
// 返回两个数中的较大值
#define LMMP_MAX(h, i) ((h) > (i) ? (h) : (i))
// 交换两个同类型变量的值
#define LMMP_SWAP(x, y, type)   \
    do {                   \
        type _swap_ = (x); \
        (x) = (y);         \
        (y) = _swap_;      \
    } while (0)
// 检查n是否为2的整数次幂
#define LMMP_POW2_Q(n) (((n) & ((n) - 1)) == 0)
// 将a向上取整为m的整数倍
#define LMMP_ROUND_UP_MULTIPLE(a, m) ((a) + (m) - 1 - ((a) + (m) - 1) % (m))

/**
 * @brief 临时内存分配函数
 * @param pmarker 栈中临时内存分配标记的头指针，用于跟踪分配的临时内存
 * @param n 要分配的字节数
 * @return 分配的内存指针
 */
void* lmmp_temp_alloc_(void** pmarker, size_t size);

/**
 * @brief 临时内存释放函数
 * @param marker 要释放的临时内存标记
 */
void lmmp_temp_free_(void* marker);

// 临时内存标记声明：用于跟踪临时内存分配
#define TEMP_DECL void* lmmp_temp_alloc_marker_ = 0
// 栈内存分配：使用alloca在栈上分配n字节内存（小内存）
#define TEMP_SALLOC(n) alloca(n)
// 堆内存分配：使用lmmp_temp_alloc_在堆上分配n字节内存（大内存）
#define TEMP_BALLOC(n) lmmp_temp_alloc_(&lmmp_temp_alloc_marker_, (n))
// 临时内存分配：小内存用栈，大内存用堆
#define TEMP_TALLOC(n) ((n) <= 0x7f00 ? TEMP_SALLOC(n) : TEMP_BALLOC(n))
// 类型化栈内存分配：分配n个type类型的栈内存
#define SALLOC_TYPE(n, type) ((type*)TEMP_SALLOC((n) * sizeof(type)))
// 类型化堆内存分配：分配n个type类型的堆内存
#define BALLOC_TYPE(n, type) ((type*)TEMP_BALLOC((n) * sizeof(type)))
// 类型化临时内存分配：智能选择栈/堆分配n个type类型内存
#define TALLOC_TYPE(n, type) ((type*)TEMP_TALLOC((n) * sizeof(type)))
// 临时内存释放：释放所有通过TEMP_BALLOC分配的临时内存
#define TEMP_FREE                                     \
    do {                                              \
        if (lmmp_temp_alloc_marker_)                  \
            lmmp_temp_free_(lmmp_temp_alloc_marker_); \
    } while (0)

// 类型化内存分配：分配n个type类型的内存（堆）
#define ALLOC_TYPE(n, type) ((type*)lmmp_alloc((size_t)(n) * sizeof(type)))
// 类型化内存重分配：将p指向的内存重分配为new_size个type类型
#define REALLOC_TYPE(p, new_size, type) ((type*)lmmp_realloc((p), (new_size) * sizeof(type)))

// 内存拷贝宏：拷贝n个limb（每个8字节），使用memmove保证重叠安全
#define lmmp_copy(dst, src, n) memmove(dst, src, (n) << 3)
// 内存置零宏：将n个limb置零（每个8字节）
#define lmmp_zero(dst, n) memset(dst, 0, (n) << 3)
// 内存设置宏：将n个limb的值设置为val（每个8字节）
#define lmmp_set(dst, val, n) memset(dst, val, (n) << 3)

// 断言宏：检查条件x是否成立，不成立则触发段错误（严格的错误检查）
// RELEASE 版本也会检查
#define lmmp_assert(x)          \
    do {                        \
        if (!(x)) {             \
            *(mp_limb_t*)0 = 0; \
        }                       \
    } while (0)

#if LAMMP_DEBUG == 1
// 调试断言宏：检查条件x是否成立，不成立则触发段错误（调试版本）
#define lmmp_debug_assert(x)                                                          \
    do {                                                                              \
        if (!(x)) {                                                                   \
            fprintf(stderr, "Assertion failed: %s:%d: %s\n", __FILE__, __LINE__, #x); \
            abort();                                                                  \
        }                                                                             \
    } while (0)
#else
// 调试断言宏：检查条件x是否成立，不成立则触发段错误（调试版本）
#define lmmp_debug_assert(x) ((void)0)
#endif

/**
 * @brief 大数加1宏（预期无进位）
 * @param p 指向大数起始位置的指针
 * @note 从最低位开始加1，直到遇到非零值（预期无进位溢出）
 */
#define lmmp_inc(p)                \
    do {                           \
        mp_ptr _p_ = (p);          \
        while (++(*(_p_++)) == 0); \
    } while (0)

/**
 * @brief 大数加指定值宏（预期无进位）
 * @param p 指向大数起始位置的指针
 * @param inc 要加的单精度数值
 * @note 先加最低位，若产生进位则逐位加1，直到无进位（预期无溢出）
 */
#define lmmp_inc_1(p, inc)             \
    do {                               \
        mp_ptr _p_ = (p);              \
        mp_limb_t _inc_ = (inc), _x_;  \
        _x_ = *_p_ + _inc_;            \
        *_p_ = _x_;                    \
        if (_x_ < _inc_)               \
            while (++(*(++_p_)) == 0); \
    } while (0)

/**
 * @brief 大数减1宏（预期无借位）
 * @param p 指向大数起始位置的指针
 * @note 从最低位开始减1，直到遇到非零值（预期无借位溢出）
 */
#define lmmp_dec(p)                \
    do {                           \
        mp_ptr _p_ = (p);          \
        while ((*(_p_++))-- == 0); \
    } while (0)

/**
 * @brief 大数减指定值宏（预期无借位）
 * @param p 指向大数起始位置的指针
 * @param dec 要减的单精度数值
 * @note 先减最低位，若产生借位则逐位减1，直到无借位（预期无溢出）
 */
#define lmmp_dec_1(p, dec)             \
    do {                               \
        mp_ptr _p_ = (p);              \
        mp_limb_t _dec_ = (dec), _x_;  \
        _x_ = *_p_;                    \
        *_p_ = _x_ - _dec_;            \
        if (_x_ < _dec_)               \
            while ((*(++_p_))-- == 0); \
    } while (0)

/**
 * @brief 大数比较函数（内联）
 * @param numa 第一个大数，长度为n
 * @param numb 第二个大数，长度为n
 * @param n 大数的单精度数(limb)长度
 * @return 1(numa>numb) / 0(numa==numb) / -1(numa<numb)
 * @note 从最高位开始逐位比较，直到找到不同位
 */
INLINE_ int lmmp_cmp_(mp_srcptr numa, mp_srcptr numb, mp_size_t n) {
    mp_ssize_t i = n;
    mp_limb_t x, y;
    while (--i >= 0) {
        x = numa[i];
        y = numb[i];
        if (x != y)
            return (x > y ? 1 : -1);
    }
    return 0;
}

/**
 * @brief 大数判零函数（内联）
 * @param p 指向大数起始位置的指针
 * @param n 大数的单精度数(limb)长度
 * @return 1(全零) / 0(非零)
 * @note 从最高位开始检查，只要有非零位则返回0
 */
INLINE_ int lmmp_zero_q_(mp_srcptr p, mp_size_t n) {
    do {
        if (p[--n] != 0)
            return 0;
    } while (n != 0);
    return 1;
}

#define LMMP_AORS_(FUNCTION, TEST)               \
    mp_limb_t _x_;                               \
    if (FUNCTION(dst, numa, numb, nb)) {         \
        do {                                     \
            if (nb >= na)                        \
                return 1;                        \
            _x_ = numa[nb];                      \
        } while (TEST);                          \
    }                                            \
    if (dst != numa && na != nb)                 \
        lmmp_copy(dst + nb, numa + nb, na - nb); \
    return 0

/**
 * @brief 大数加法静态内联函数 [dst,na]=[numa,na]+[numb,nb]
 * @param dst 输出结果缓冲区，存储numa + numb
 * @param numa 第一个加数，长度为na
 * @param na 第一个加数的 limb 长度
 * @param numb 第二个加数，长度为nb
 * @param nb 第二个加数的 limb 长度
 * @return 进位标志（1表示有进位，0表示无进位）
 * @warning 0<nb<=na, eqsep(dst,[numa|numb])
 */
INLINE_ mp_limb_t lmmp_add_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_srcptr numb, mp_size_t nb) {
    LMMP_AORS_(lmmp_add_n_, ((dst[nb++] = _x_ + 1) == 0));
}

/**
 * @brief 大数减法静态内联函数 [dst,na]=[numa,na]-[numb,nb]
 * @param dst 输出结果缓冲区，存储numa - numb
 * @param numa 被减数，长度为na
 * @param na 被减数的 limb 长度
 * @param numb 减数，长度为nb
 * @param nb 减数的 limb 长度
 * @return 借位标志（1表示有借位，0表示无借位）
 * @warning 0<nb<=na, eqsep(dst,[numa|numb])
 */
INLINE_ mp_limb_t lmmp_sub_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_srcptr numb, mp_size_t nb) {
    LMMP_AORS_(lmmp_sub_n_, ((dst[nb++] = _x_ - 1), _x_ == 0));
}

#undef LMMP_AORS_

// 单精度加减运算通用宏：封装单精度加减的公共逻辑
#define LMMP_AORS_1_(OP, CB)                        \
    mp_size_t _i_ = 1;                              \
    mp_limb_t _x_ = numa[0], _r_ = _x_ OP x;        \
    dst[0] = _r_;                                   \
    if (CB(_r_, _x_, x)) {                          \
        do {                                        \
            if (_i_ >= na)                          \
                return 1;                           \
            _x_ = numa[_i_];                        \
            _r_ = _x_ OP 1;                         \
            dst[_i_] = _r_;                         \
            ++_i_;                                  \
        } while (CB(_r_, _x_, 1));                  \
    }                                               \
    if (numa != dst && na != _i_)                   \
        lmmp_copy(dst + _i_, numa + _i_, na - _i_); \
    return 0

// 加法进位判断宏：判断加法是否产生进位
#define LMMP_ADDCB_(r, x, y) ((r) < (y))
// 减法借位判断宏：判断减法是否产生借位
#define LMMP_SUBCB_(r, x, y) ((x) < (y))

/**
 * @brief 大数加单精度数静态内联函数 [dst,na]=[numa,na]+x
 * @param dst 输出结果缓冲区，存储numa + x
 * @param numa 被加数，长度为na
 * @param na 被加数的 limb 长度
 * @param x 加数（单个 limb ）
 * @return 进位标志（1表示有进位，0表示无进位）
 * @warning na>0, eqsep(dst,numa)
 */
INLINE_ mp_limb_t lmmp_add_1_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_limb_t x) { LMMP_AORS_1_(+, LMMP_ADDCB_); }

/**
 * @brief 大数减单精度数静态内联函数 [dst,na]=[numa,na]-x
 * @param dst 输出结果缓冲区，存储numa - x
 * @param numa 被减数，长度为na
 * @param na 被减数的 limb 长度
 * @param x 减数（单个 limb ）
 * @return 借位标志（1表示有借位，0表示无借位）
 * @warning na>0, eqsep(dst,numa)
 */
INLINE_ mp_limb_t lmmp_sub_1_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_limb_t x) { LMMP_AORS_1_(-, LMMP_SUBCB_); }

/**
 * @brief 计算大数转换为字符串，字符串需要的缓冲区长度
 * @param numa 输入大数，长度为na
 * @param na 大数的 limb 长度
 * @param base 目标基数（2~256）
 * @return 大数在指定基数下的位数
 * @warning na>=0, 2<=base<=256
 * @note 将会忽略numa的前导零，
 *       1. if (numa!=NULL) 返回的长度可能会多分配一个字符空间
 *       2. if (numa==NULL) 返回na个limb长度的数的最大可能字符长度（最坏情况）
 */
INLINE_ mp_size_t lmmp_to_str_len_(mp_srcptr numa, mp_size_t na, int base) {
    int mslbits = 0;
    if (numa) {
        do {
            if (na == 0)
                return 1;
        } while (numa[--na] == 0);
        mslbits = lmmp_limb_bits_(numa[na]);
    }
    return lmmp_mulh_(na * LIMB_BITS + mslbits, lmmp_bases_[base].inv_lg_base) + 1;
}

/**
 * @brief 计算字符串转大数所需的 limb 缓冲区长度
 * @param src 输入字符串指针
 * @param len 字符串长度
 * @param base 字符串的基数（2~256）
 * @return 存储该字符串数值所需的 limb 缓冲区长度
 * @warning len>=0, 2<=base<=256
 * @note 将会忽略非零字符，
 *       1. if (src!=NULL) 返回的长度可能会多分配一个 limb 空间
 *       2. if (src==NULL) 返回len位base进制数的最大可能 limb 长度（最坏情况）
 */
INLINE_ mp_size_t lmmp_form_str_len_(const mp_byte_t* src, mp_size_t len, int base) {
    if (src) {
        do {
            if (len == 0)
                return 1;
        } while (src[--len] == 0);
        ++len;
    }
    return lmmp_mulh_(len, lmmp_bases_[base].lg_base) + 1;
}

#ifdef __cplusplus
}  // extern "C"
#endif

#undef LMMP_ADDCB_
#undef LMMP_SUBCB_
#undef LMMP_AORS_1_


#undef INLINE_

#endif  // LAMMP_LMMPN_H