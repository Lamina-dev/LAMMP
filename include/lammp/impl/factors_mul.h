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

#ifndef __LAMMP_FACTORS_MUL_H__
#define __LAMMP_FACTORS_MUL_H__

#include "../numth.h"

typedef struct factor {
    uint f;
    uint j;
} factor;

typedef struct factor* factors;

/**
 * @brief 计算因子的累乘，并将结果放入dst中
 * @param dst 结果数组
 * @param rn 结果数组的长度
 * @param fac 因子数组
 * @param nfactors 因子数组的长度
 * @param N 因子的最大值（或最大范围）
 * @warning 因子必须要小于N，且因子必须要单调递增，且不重复，因子的贡献必须要大于0。
 *          因子数组必须为小因子大指数形式，可以存在大的因子有较大的指数，但整体的趋势必须是小因子大指数。
 *          暂不清楚其最差可以接受至何种形式的因子数组。在组合数以及由阶乘和幂次构成的有理数中，未见不满足
 *          此条件的例子。且需要注意的是，因子数组中，指数最大的底数不可超过0xffff。
 * @return 结果数组的长度
 */
mp_size_t lmmp_factors_mul_(mp_ptr dst, mp_size_t rn, const factors fac, uint nfactors, uint N);

#endif // __LAMMP_FACTORS_MUL_H__