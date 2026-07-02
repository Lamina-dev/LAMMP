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
#ifndef __LAMMP_LOG2_EXP2_H__
#define __LAMMP_LOG2_EXP2_H__

#include <stdint.h>

/**
 * @brief floor(log2(1+x/B)*B), B=2^64
 * @param x 输入的小数部分
 * @return floor(log2(1+x/B)*B)
 */
uint64_t log2_fixed_64(uint64_t x);

/**
 * @brief floor(exp2(x/B)*B-B), B=2^64
 * @param x 输入的小数部分
 * @return floor(exp2(x/B)*B-B)
 */
uint64_t exp2_fixed_64(uint64_t x);

/**
 * @brief floor(log2(1+x/B)*B), B=2^128
 * @param high 输入的小数部分高64位
 * @param low 输入的小数部分低64位
 * @param dst 输出数组（2个元素），floor(log2(1+x/B)*B) 的高64位，低64位
 * @note x = high * 2^64 + low
 */
void log2_fixed_128(uint64_t* dst, uint64_t high, uint64_t low);

/**
 * @brief floor(exp2(x/B)*B-B), B=2^128
 * @param high 输入的小数部分高64位
 * @param low 输入的小数部分低64位
 * @param dst 输出数组（2个元素），floor(exp2(x/B)*B-B) 的高64位，低64位
 * @note x = high * 2^64 + low
 */
void exp2_fixed_128(uint64_t* dst, uint64_t high, uint64_t low);

#endif // __LAMMP_LOG2_EXP2_H__