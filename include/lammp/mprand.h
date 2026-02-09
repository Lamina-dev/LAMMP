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

#ifndef LAMMP_MPRAND_H
#define LAMMP_MPRAND_H

#include "lmmp.h"

/*
 LAMMP 主要实现了两个随机数引擎：PCG-XSL-RR-128/64 和 xorshift256++。
 其中，后者通常为默认的lammp的随机数引擎，生成速度较前者快，大约有2.5倍性能差距。
 可以参考以下文章了解PCG：<https://www.pcg-random.org/paper.html>

 PCG网站<https://www.pcg-random.org>中也有PCG和xorshift256++及其他随机生成器的比较。

 PCG-XSL-RR-128/64 生成的随机数序列通常拥有比xorshift256++更好的统计性能，当然两者的统计性能
 在大部分场景下都足够的好。

 备注：两者通常都比cpp标准库中的梅森旋转算法（Mersenne Twister）生成的速度更快。
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化全局随机数生成器
 * @param seed 种子
 * @param seed_type 随机数发生器类型（0：pcg64_128，1：xorshift256++）
 * @warning seed_type 必须在0到1之间，如果不是，则使用 seed_type%2 作为seed_type，
 *          如果全局种子已经初始化，则会更新全局种子，或更新发生器类型
 * @note 由于使用int类型作为种子，我们将会维护一个全局种子随机序列，丰富种子的随机性。
 */
void lmmp_global_rng_init_(int seed, int seed_type);

/**
 * @brief 生成随机大整数（0 - B^n-1 均匀分布）
 * @param dst 随机数存储位置
 * @param n dst的 limb 长度
 * @param seed 种子（建议使用有效位数较多的随机数，以保证随机数质量）
 * @param seed_type 随机数发生器类型（0：pcg64_128，1：xorshift256++）
 * @warning seed_type 必须在0到1之间，如果不是，则使用 seed_type%2 作为seed_type
 * @return 随机数的 limb 长度（这是由于可能存在生成随机数为0的情况（虽然几乎不可能），
 *         所以返回值可能小于n，但不会大于n）
 */
mp_size_t lmmp_seed_random_(mp_ptr dst, mp_size_t n, mp_limb_t seed, int seed_type);

/**
 * @brief 生成随机大整数（0 - B^n-1 均匀分布）
 * @param dst 随机数存储位置
 * @param n dst的 limb 长度
 * @warning 种子由 lmmp_global_rng_init() 设置，发生器类型由 lmmp_global_rng_init() 设置
 *          如果没有进行全局初始化，则使用默认种子（默认等价设置全局种子为0，并不代表全局种子为0）
 *          和默认发生器类型（默认为xorshift256++），即未设置全局种子，行为等价于 lmmp_global_rng_init_(0, 1)
 * @note 每调用一次此函数，种子将会进行一次更新，以确保多次调用时的种子不同，但是只要每次调用的方式
 *       和顺序相同，在同一个进程中，每次生成的随机数序列相同。
 * @return 随机数的 limb 长度（由于可能存在生成随机数为0的情况，所以返回值可能小于n，但不会大于n）
 */
mp_size_t lmmp_random_(mp_ptr dst, mp_size_t n);

#ifdef __cplusplus
}
#endif

#endif // LAMMP_MPRAND_H
