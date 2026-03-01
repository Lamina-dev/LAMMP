#include "../../../include/lammp/impl/heap.h"

static inline void swap_num_node(num_node_ptr a, num_node_ptr b) {
    num_node temp = *a;
    *a = *b;
    *b = temp;
}

// 向上调整最小堆（插入元素后维护堆性质：len小的在上）
static void heapifyUp(num_heap* pq, size_t index) {
    size_t parent = (index - 1) / 2; 

    // 最小堆：当前节点len < 父节点len 时交换
    while (index > 0 && pq->head[index].n < pq->head[parent].n) {
        swap_num_node(&pq->head[index], &pq->head[parent]);
        index = parent;
        parent = (index - 1) / 2;
    }
}

// 向下调整最小堆
static void heapifyDown(num_heap* pq, size_t index) {
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
            break;  // 堆性质已满足，退出循环
        }

        swap_num_node(&pq->head[index], &pq->head[smallest]);
        index = smallest;  // 继续向下调整
    }
}

void lmmp_num_heap_push_(num_heap* pq, mp_ptr elem, mp_size_t n) {
    lmmp_param_assert(pq->size < pq->cap);
    pq->head[pq->size].num = elem;
    pq->head[pq->size].n = n;
    pq->size++;
    heapifyUp(pq, pq->size - 1);
}

bool lmmp_num_heap_pop_(num_heap* pq, num_node_ptr outElem) {
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

mp_ptr lmmp_num_heap_mul_(num_heap* pq, mp_size_t* rn) {
    num_node numa, numb;
    numa.num = NULL;
    numb.num = NULL;
    numa.n = 0;
    numb.n = 0;

    while (lmmp_num_heap_pop_(pq, &numa)) {
        if (!lmmp_num_heap_pop_(pq, &numb)) {
            break;
        }
        mp_ptr prod = ALLOC_TYPE(numa.n + numb.n, mp_limb_t);
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