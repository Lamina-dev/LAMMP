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

// 阶乘计算中，大于此阈值的质数幂将使用快速幂算法
#define FACTORIAL_PRIME_POW_THRESHOLD 20

// 阶乘计算中，朴素连乘的乘法空间长度阈值
#define FACTORIAL_MUL_MAX_THRESHOLD 20

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
#define lmmp_pow_1_size_(base, exp) (( ceil((double)(exp) * log2((double)base)) + LIMB_BITS - 1 ) / LIMB_BITS + 2)

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
 * @warning base>1, sep[dst|base], exp>0
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

#define PRI_MMP_ZERO 3 // 位图的初始化值 11000000...

/**
 * @brief 计算素数表大小
 * @param n 初始化不超过 n 的素数表
 * @return 素数表大小（高估素数数量，不会高估太多）
 */
INLINE_ size_t lmmp_prime_size_(ulong n) {
    /*
     * 这是一个不会低估的素数计数估计函数，使用了一些经验数据，其估计的数据可以参考：
     * 总样本数: 9800001（500000000-10000000 之间以 50 为步长）
     * 平均相对误差: 0.0830378%
     * 最大相对误差: 0.123542%
     * 最小相对误差: 0.0565845%
     * 平均绝对误差: 11190.7
     * 最大绝对误差: 22288
     * 低估次数: 0 (0%)
     */
    if (n < 50) {
        return (double)n / 3 + 2;
    } else if (n < 500000) {
        return ceil(1.002 * (double)n / (log(n) - 1.095));
    } else if (n < 2500000) {
        return ceil((double)n / (log(n) - 1.095));
    } else if (n < 10000000) {
        return ceil((double)n / (log(n) - 1.087));
    } else if (n < 100000000) {
        return ceil((double)n / (log(n) - 1.085));
    } else {
        return ceil((double)n / (log(n) - 1.075));
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

typedef struct num_node {
    mp_ptr num;
    mp_size_t n;
} num_node;

typedef num_node* num_node_ptr;

typedef struct num_heap {
    num_node_ptr head;
    size_t size;
    size_t cap;
} num_heap;

/**
 * @brief 初始化优先队列
 * @param pq 优先队列指针
 * @param capa 优先队列容量
 */
INLINE_ void lmmp_num_heap_init_(num_heap* pq, size_t capa) {
    pq->head = ALLOC_TYPE(capa, num_node);
    for (size_t i = 0; i < capa; ++i) {
        pq->head[i].num = NULL;
        pq->head[i].n = 0;
    }
    pq->cap = capa;
    pq->size = 0;
}

/**
 * @brief 释放优先队列
 * @param pq 优先队列指针
 */
INLINE_ void lmmp_num_heap_free_(num_heap* pq) {
    lmmp_debug_assert(pq->size == 0);
    lmmp_free(pq->head);
    pq->cap = 0;
    pq->size = 0;
    pq->head = NULL;
}

/**
 * @brief 入队
 * @param pq 优先队列指针
 * @param elem 待入队的元素指针
 * @param n 元素的 limb 长度
 */
void lmmp_num_heap_push_(num_heap* pq, mp_ptr elem, mp_size_t n);

/**
 * @brief 出队
 * @param pq 优先队列指针
 * @param elem 出队的元素指针
 * @return 若队列为空，返回 false，否则返回 true
 */
bool lmmp_num_heap_pop_(num_heap* pq, num_node_ptr elem);

/**
 * @brief 计算 n! 阶乘的 limb 缓冲区长度
 * @param n 阶乘的阶数
 * @return n! 阶乘的 limb 缓冲区长度（比实际长度多 1-2 个 limb）
 */
INLINE_ mp_size_t lmmp_factorial_size_(uint n) {
    double ln_fact = lgamma(n + 1.0);       
    double log2_fact = ln_fact / log(2.0);
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
mp_size_t lmmp_factorial_(mp_ptr dst, mp_size_t rn, uint n);

#ifdef INLINE_
#undef INLINE_
#endif // INLINE_

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // LAMMP_NUMTH_H