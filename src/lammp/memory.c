#include "../../include/lammp/lmmpn.h"

#define HSIZE sizeof(void*)

/*
 *
 * 分配顺序：p1 → p2 → p3 → ...
 * 链表方向：最新分配的指向之前的
 *
 *  ┌─────────────┐
 *  │   marker    │
 *  │  (stack)    │
 *  └──────┬──────┘
 *         │
 *         ▼
 *    ┌──────────┬───┬──────────┬───┬──────────┬───┐
 *    │ next: p2 ├──►│ next: p1 ├──►│ next:NULL│   │  ← HSIZE
 *    │          │   │          │   │          │   │
 *    ├──────────┤   ├──────────┤   ├──────────┤   │
 *    │          │   │          │   │          │   │
 *    │   (d3)   │   │   (d2)   │   │   (d1)   │   │  ← DATA
 *    │          │   │          │   │          │   │
 *    └──────────┴───┴──────────┴───┴──────────┴───┘
 *         ▲              ▲              ▲
 *         │              │              │
 *         └──────────────┴──────────────┘
 *                  返回给调用者的指针
 *
 *
 *   marker
 *     │
 *     ▼
 *   ┌─────┐   ┌─────┐   ┌─────┐
 *   │ p3  ├──►│ p2  ├──►│ p1  ├──►NULL
 *   ├─────┤   ├─────┤   ├─────┤
 *   │ d3  │   │ d2  │   │ d1  │
 *   └─────┘   └─────┘   └─────┘
 *
 * lmmp_temp_alloc_: 分配 size + HSIZE 字节
 *    - 在头部存储当前链表头指针
 *    - 更新 marker 指向新块
 *    - 返回指向数据区域的指针
 *
 * lmmp_temp_free_: 从 marker 开始遍历链表
 *    - 读取 next 指针
 *    - 释放当前块
 *    - 继续处理下一块
 */

void* lmmp_temp_alloc_(void** pmarker, size_t size) {
/*
 * pmarker is a head pointer to a linked list of allocated memory blocks.
 * Each allocated block has a header of size HSIZE, which is used to store the
 * next pointer of the block. The actual data starts at (mp_byte_t*)p + HSIZE.
 */
    void* p = lmmp_alloc(size + HSIZE);
    *(void**)p = *pmarker;
    *pmarker = p;
    return (mp_byte_t*)p + HSIZE;
}

void lmmp_temp_free_(void* marker) {
/*
 *  Free all allocated memory blocks in the linked list pointed to by pmarker.
 */
    while (marker) {
        void* next = *(void**)marker;
        lmmp_free(marker);
        marker = next;
    }
}
#if MEMORY_CHECK == 1
#else
void* lmmp_alloc(size_t size) {
    if (size) {
        void* ret = malloc(size);
        lmmp_debug_assert(ret != NULL);
        lmmp_assert(ret);
        return ret;
    }
    return NULL;
}
#endif

#if MEMORY_CHECK == 1
#else
void* lmmp_realloc(void* oldptr, size_t new_size) {
    void* ret = realloc(oldptr, new_size);
    lmmp_debug_assert(ret != NULL);
    lmmp_assert(ret);
    return ret;
}
#endif

#if MEMORY_CHECK == 1
#else
void lmmp_free(void* ptr) { 
    if (ptr) 
        free(ptr); 
}
#endif
