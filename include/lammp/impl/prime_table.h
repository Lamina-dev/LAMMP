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
 * @brief 初始化全局素数表
 * @param n 素数表大小
 */
void lmmp_prime_int_table_init_(uint n);

/**
 * @brief 释放全局素数表
 */
void lmmp_prime_int_table_free_(void);

#endif  // __LAMMP_PRIME_TABLE_H__