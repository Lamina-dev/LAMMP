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

#ifndef __LAMMP_ELE_MUL_H__
#define __LAMMP_ELE_MUL_H__

#include "../lmmpn.h"
#include "../numth.h"
#include "tmp_alloc.h"

#ifndef INLINE_
#define INLINE_ static inline
#endif

/*
FIXME: 这里的优先队列乘法可以进一步优化，目前的实现，每次合并
都需要调用一次lmmp_alloc，我们可以通过预先构建哈夫曼树，使用
两块内存交替来使得计算乘法时，每个节点内存恰好不重叠，并刚好把
结果写入到我们需要的内存块。
同时另一个优化时，在初始化入队时，将所有元素的低位0全部移除，即
保证奇数相乘，最后一次性移位完成。

TODO: 需要新增一个计算多个mp_ptr累乘的函数 
*/


typedef struct num_node {
    mp_ptr num;
    mp_size_t n;
} num_node;

typedef num_node* num_node_ptr;

typedef struct num_heap {
    num_node_ptr restrict head;
    size_t size;
    size_t cap;
} num_heap;

/**
 * @brief 初始化优先队列
 * @param pq 优先队列指针
 * @param capa 优先队列容量
 */
INLINE_ void lmmp_num_heap_init_(num_heap* restrict pq, size_t capa) {
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
INLINE_ void lmmp_num_heap_free_(num_heap* restrict pq) {
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
 * @brief 将队列中所有元素相乘
 * @param pq 优先队列指针
 * @param rn 结果指针的 limb 长度
 * @warning 队列非空，pq!=NULL
 * @note 返回的指针必须手动释放，原队列中的元素指针都将被释放
 * @return 结果指针
 */
mp_ptr lmmp_num_heap_mul_(num_heap* pq, mp_size_t* rn);

/**
 * @brief 计算limbs数组的累乘积
 * @param dst 结果指针（长度为 n 个 limb）
 * @param limbs 数组指针
 * @param n limbs数组长度
 * @param tp 临时指针（长度为 n 个 limb）
 * @warning dst!=NULL, limbs!=NULL, n>0, tp!=NULL
 * @return 结果指针的 limb 长度
 */
mp_size_t lmmp_elem_mul_ulong_(mp_ptr dst, const ulongp limbs, mp_size_t n, mp_ptr tp);

typedef struct fac_t {
    uint f; // factor
    uint j; // exp
} fac_t;

typedef fac_t* fac_ptr;
typedef const fac_t* fac_srcptr;

/**
 * @brief 计算因子的累乘，并将结果放入dst中
 * @param dst 结果数组
 * @param rn 结果数组的长度
 * @param fac 因子数组（将会被递归覆盖）
 * @param nfactors 因子数组的长度
 * @warning 因子必须要小于N，且因子必须要单调递增，且不重复，因子的贡献必须要大于0。
 *          因子数组必须为小因子大指数形式，可以存在大的因子有较大的指数，但整体的趋势必须是小因子大指数。
 *          暂不清楚其最差可以接受至何种形式的因子数组。在组合数以及由阶乘和幂次构成的有理数中，未见不满足
 *          此条件的例子。
 * @return 结果数组的长度
 */
mp_size_t lmmp_factors_mul_(mp_ptr dst, mp_size_t rn, fac_ptr fac, uint nfactors);

#undef INLINE_

#endif // __LAMMP_ELE_MUL_H__