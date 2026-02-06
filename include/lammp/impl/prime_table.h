#ifndef __LAMMP_PRIME_TABLE_H__
#define __LAMMP_PRIME_TABLE_H__
#include "../numth.h"

#define CHAR_PRIME_size 54

static const uint8_t pri_char_table[54] = {2,   3,   5,   7,   11,  13,  17,  19,  23,  29,  31,  37,  41,  43,
                                           47,  53,  59,  61,  67,  71,  73,  79,  83,  89,  97,  101, 103, 107,
                                           109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181,
                                           191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251};

typedef struct prime_short {
    ushortp pri;     // prime 数组指针
    ulongp mmp;      // prime 位图指针（1为合数，0为素数）
    ushort prin;     // prime 数量
    ushort mmp_cal;  // prime 位图容量
    ushort N;        // 不超过 N 的素数表
} pri_short;

typedef struct prime_int {
    uintp pri;     // prime 数组指针
    ulongp mmp;    // prime 位图指针（1为合数，0为素数）
    uint prin;     // prime 数量
    uint mmp_cal;  // prime 位图容量
    uint N;        // 不超过 N 的素数表
} pri_int;

#define PRI_MMP_ZERO 3  // 位图的初始化值 11000000...

#ifndef INLINE_
#define INLINE_ static inline
#endif // INLINE_

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

#ifdef INLINE_
#undef INLINE_
#endif // INLINE_

#endif  // __LAMMP_PRIME_TABLE_H__