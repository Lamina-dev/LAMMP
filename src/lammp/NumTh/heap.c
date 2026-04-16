/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../../../include/lammp/impl/heap.h"
#include "../../../include/lammp/impl/mparam.h"
#include "../../../include/lammp/impl/mpdef.h"

static inline void swap_num_node(num_node_ptr restrict a, num_node_ptr restrict b) {
    num_node temp = *a;
    *a = *b;
    *b = temp;
}

// 向上调整最小堆（插入元素后维护堆性质：len小的在上）
static inline void heapifyUp(num_heap* restrict pq, size_t index) {
    size_t parent = (index - 1) / 2; 

    // 最小堆：当前节点len < 父节点len 时交换
    while (index > 0 && pq->head[index].n < pq->head[parent].n) {
        swap_num_node(&pq->head[index], &pq->head[parent]);
        index = parent;
        parent = (index - 1) / 2;
    }
}

// 向下调整最小堆
static inline void heapifyDown(num_heap* restrict pq, size_t index) {
    if (pq->size == 0) {
        return;
    }
    lmmp_debug_assert(index < pq->size);

    while (1) {
        size_t smallest = index;
        size_t left = 2 * index + 1;
        size_t right = 2 * index + 2;

        if (left < pq->size && pq->head[left].n < pq->head[smallest].n) {
            smallest = left;
        }
        if (right < pq->size && pq->head[right].n < pq->head[smallest].n) {
            smallest = right;
        }

        if (smallest == index) {
            break; 
        }

        swap_num_node(&pq->head[index], &pq->head[smallest]);
        index = smallest; 
    }
}

void lmmp_num_heap_push_(num_heap* restrict pq, mp_ptr elem, mp_size_t n) {
    lmmp_param_assert(pq->size < pq->cap);
    pq->head[pq->size].num = elem;
    pq->head[pq->size].n = n;
    pq->size++;
    heapifyUp(pq, pq->size - 1);
}

bool lmmp_num_heap_pop_(num_heap* restrict pq, num_node_ptr restrict outElem) {
    if (pq->size == 0) {
        outElem->num = NULL;
        outElem->n = 0;
        return false;
    }
    *outElem = pq->head[0];
    pq->head[0] = pq->head[pq->size - 1];
    --(pq->size);
    heapifyDown(pq, 0);
    return true;
}

mp_ptr lmmp_num_heap_mul_(num_heap* restrict pq, mp_size_t* restrict rn) {
    num_node numa, numb;
    numa.num = NULL;
    numb.num = NULL;
    numa.n = 0;
    numb.n = 0;

    while (lmmp_num_heap_pop_(pq, &numa)) {
        if (!lmmp_num_heap_pop_(pq, &numb)) {
            break;
        }
        mp_ptr restrict prod = ALLOC_TYPE(numa.n + numb.n, mp_limb_t);
        lmmp_mul_(prod, numb.num, numb.n, numa.num, numa.n);
        lmmp_free(numa.num);
        lmmp_free(numb.num);
        numa.num = prod;
        numa.n += numb.n;
        numa.n -= numa.num[numa.n - 1] == 0 ? 1 : 0;
        lmmp_num_heap_push_(pq, numa.num, numa.n);
    }
    *rn = numa.n;
    return numa.num;
}

mp_size_t lmmp_elem_mul_ulong_(mp_ptr restrict dst, const ulongp restrict limbs, mp_size_t n, mp_ptr restrict tp) {
    if (n < ELEM_MUL_BASECASE_THRESHOLD) {
        lmmp_debug_assert(n > 0);
        dst[0] = limbs[0];
        mp_size_t rn = 1;
        for (mp_size_t i = 1; i < n; i++) {
            dst[rn] = lmmp_mul_1_(dst, dst, rn, limbs[i]);
            rn++;
            rn -= dst[rn - 1] == 0 ? 1 : 0;
        }
        return rn;
    }
    mp_size_t halfn = n / 2;
    mp_size_t n1 = lmmp_elem_mul_ulong_(tp, limbs, halfn, dst);
    mp_size_t n2 = lmmp_elem_mul_ulong_(tp + halfn, limbs + halfn, n - halfn, dst + halfn);
    if (n1 > n2)
        lmmp_mul_(dst, tp, n1, tp + halfn, n2);
    else
        lmmp_mul_(dst, tp + halfn, n2, tp, n1);
    n = n1 + n2;
    n -= dst[n - 1] == 0 ? 1 : 0;
    return n;
}
