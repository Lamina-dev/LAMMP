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
#ifndef __LAMMP_RAND_STATE_H__
#define __LAMMP_RAND_STATE_H__

#include "../lmmp.h"

typedef struct {
    mp_limb_t state[2];
    mp_limb_t inc[2];  // 必须为奇数
} pcg64_128_state;

typedef struct {
    mp_limb_t s[4];  // 256位状态，必须初始化为非零值
} xorshift256pp_state;

/**
 * @brief 初始化pcg64_128随机数生成器
 * @param rng 随机数生成器状态
 * @warning rng != NULL
 * @param seed 种子
 */
void lmmp_pcg64_128_srandom(pcg64_128_state* rng, mp_limb_t seed);

/**
 * @brief 生成一个随机数
 * @param rng 随机数生成器状态
 * @warning rng != NULL
 * @return 随机数
 */
mp_limb_t lmmp_pcg64_128_random(pcg64_128_state* rng);

/**
 * @brief 初始化xorshift256++随机数生成器
 * @param rng 随机数生成器状态
 * @warning rng != NULL
 * @param seed 种子
 */
void lmmp_xorshift_srandom(xorshift256pp_state* rng, mp_limb_t seed);

/**
 * @brief 生成一个随机数
 * @param rng 随机数生成器状态
 * @warning rng != NULL
 * @return 随机数
 */
mp_limb_t lmmp_xorshift_random(xorshift256pp_state* rng);

#endif // __LAMMP_RAND_STATE_H__