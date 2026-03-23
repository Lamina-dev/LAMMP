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

#ifndef __LAMMP_HEAP_H__
#define __LAMMP_HEAP_H__
#include "../lmmpn.h"

#ifndef INLINE_
#define INLINE_ static inline
#endif

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

#undef INLINE_

#endif // __LAMMP_HEAP_H__