#ifndef __LAMMP_PRIME_TABLE_H__
#define __LAMMP_PRIME_TABLE_H__
#include "../numth.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PRIME_SHORT_TABLE_SIZE 6542

#define PRIME_SHORT_TABLE_N 0x10000

extern const ushort prime_short_table[PRIME_SHORT_TABLE_SIZE];

bool lmmp_is_prime_table(uint p);

uint lmmp_get_nth_prime_table(uint n);


/**
 * @brief 计算 n 范围内的short素数数量
 * @param n 范围
 * @return 素数数量
 */
ushort short_prime_count(ushort n);

/**
 * @brief 计算 n 范围内的素数数量
 * @param n 范围
 * @note 不会低估素数数量，可能恰好超过 pi(n)
 * @return 素数数量
 */
ulong lmmp_prime_size_(ulong n);

typedef struct prime_int {
    uintp p;       // prime 数组指针（仅存储大于65536的素数）
    ulongp m;      // prime 位图指针（1为质数，0为合数）
    uint n;        // prime 数量（当前p数组中素数数量）
    uint m_size;   // prime 位图容量
    uint N;        // 位图记录的最大值 N
    uint pN;       // prime 数组当前记录的是不超过 pN 的素数
} prime_int;

extern prime_int global_prime_int_table;

static inline uint lmmp_get_prime_count_table(uint n) {
    if (n <= PRIME_SHORT_TABLE_N) {
        return short_prime_count(n);
    } else {
        lmmp_debug_assert(n <= global_prime_int_table.N);
        return global_prime_int_table.n + PRIME_SHORT_TABLE_SIZE;
    }
}

/**
 * @brief 初始化全局素数表
 * @param n 素数表大小
 * @param init_primes 是否初始化素数数组
 */
void lmmp_prime_int_table_init_(uint n, bool init_primes);

/**
 * @brief 更新全局素数表（利用已经生成的位图）
 * @param n 将素数数组扩展到不超过 n 的素数（如果没有更新过的话）
 * @warning 该函数不会更新位图，如果 n 超过了位图记录的最大值 N，则会直接返回该函数，无任何操作
 */
void lmmp_prime_int_table_update_(uint n);

/**
 * @brief 释放全局素数表
 */
static inline void lmmp_prime_int_table_free_(void) {
    if (global_prime_int_table.p != NULL)
        lmmp_free(global_prime_int_table.p);
    if (global_prime_int_table.m != NULL)
        lmmp_free(global_prime_int_table.m);
    global_prime_int_table.m = NULL;
    global_prime_int_table.p = NULL;
    global_prime_int_table.N = 0;
    global_prime_int_table.m = 0;
    global_prime_int_table.m_size = 0;
}

#ifdef __cplusplus
}
#endif

#endif  // __LAMMP_PRIME_TABLE_H__