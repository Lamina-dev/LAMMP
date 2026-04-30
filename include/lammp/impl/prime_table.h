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

#ifndef __LAMMP_PRIME_TABLE_H__
#define __LAMMP_PRIME_TABLE_H__
#include "../numth.h"

#define PRIME_SHORT_TABLE_SIZE 6542

#define PRIME_SHORT_TABLE_N 0x10000

extern const ushort prime_short_table[PRIME_SHORT_TABLE_SIZE];

/**
 * @brief 根据全局素数表判断一个数是否为素数
 * @param p 待判断的数
 * @warning p>1, p%2==1
 * @note 若 p 超过了当前全局素数表的范围，则会触发 debug_assert
 * @return true 素数，false 合数
 */
bool lmmp_is_prime_table_(uint p);

/**
 * @brief 计算小于等于 n 的素数数量
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
 * @brief 初始化全局素数表
 * @param n 素数表大小
 */
void lmmp_prime_int_table_init_(uint n);

/**
 * @brief 释放全局素数表
 */
void lmmp_prime_int_table_free_(void);

typedef struct {
    uintp pp;       // 仅存储奇素数
    uint size;      // pp 数组大小
    uint start_idx; // 位图下一次解析的起始索引
    int is_end;     // 是否已经遍历到全局质数表末尾
} prime_cache_t;

/*
 * 示例代码（遍历全局奇素数表）：
 *
    lmmp_prime_int_table_init_(n);
    prime_cache_t cache;
    lmmp_prime_cache_init_(&cache);
    while (cache.is_end == 0) {
        lmmp_prime_cache_next_(&cache);
        for (uint i = 0; i < cache.size; i++) {
            ....
        }
    }
    lmmp_prime_cache_free_(&cache);
*/

/**
 * @brief 初始化素数表缓存
 * @param cache 缓存结构体
 */
void lmmp_prime_cache_init_(prime_cache_t* cache);

/**
 * @brief 素数表缓存更新（从小到大遍历全局质数表）
 * @param cache 缓存结构体
 * @note 缓存只存储奇素数，当遍历到全局质数表末尾时，is_end 置为 1
 */
void lmmp_prime_cache_next_(prime_cache_t* cache);

/**
 * @brief 释放素数表缓存
 * @param cache 缓存结构体
 */
void lmmp_prime_cache_free_(prime_cache_t* cache);

#endif  // __LAMMP_PRIME_TABLE_H__