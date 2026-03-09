#ifndef __LAMMP_PRIME_TABLE_H__
#define __LAMMP_PRIME_TABLE_H__
#include "../numth.h"

#define PRIME_SHORT_TABLE_SIZE 6542

#define PRIME_SHORT_TABLE_N 0x10000

extern const ushort prime_short_table[PRIME_SHORT_TABLE_SIZE];

/**
 * @brief 根据全局素数表判断一个数是否为素数
 * @param p 待判断的数
 * @warning 若 p 超过了当前全局素数表的范围，则会触发 debug_assert
 * @return true 素数，false 合数
 */
bool lmmp_is_prime_table_(uint p);

/**
 * @brief 根据全局素数表获取第 n 个素数
 * @param n 第 n 个素数
 * @warning 若 n 超过了当前全局素数表的范围，则会触发 debug_assert
 * @note 第 n 个素数的索引从 0 开始，即第 0 个素数为 2
 * @return 第 n 个素数
 */
uint lmmp_nth_prime_table_(uint n);

/**
 * @brief 计算 n 范围内的short素数数量
 * @param n 范围
 * @return 素数数量
 */
ushort lmmp_prime_cnt16_(ushort n);

/**
 * @brief 估计 n 范围内的素数数量
 * @param n 范围
 * @note 不会低估素数数量，可能恰好超过 pi(n)，用以估计素数数组需要的空间
 * @return 素数数量
 */
ulong lmmp_prime_size_(ulong n);

/**
 * @brief 返回当前全局质数表的质数数组记录的最大的n
 */
uint lmmp_prime_cnt_table_(uint n);

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
void lmmp_prime_int_table_free_(void);

#endif  // __LAMMP_PRIME_TABLE_H__