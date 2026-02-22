#include "../../include/lammp/lmmpn.h"
#undef lmmp_alloc
#undef lmmp_realloc
#undef lmmp_free
#include "../../include/lammp/impl/safe_memory.h"

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
#if MEMORY_CHECK == 1
    void* p = lmmp_alloc(size + HSIZE, __FILE__, __LINE__);
#else
    void* p = lmmp_alloc(size + HSIZE);
#endif
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
#if MEMORY_CHECK == 1
        lmmp_free(marker, __FILE__, __LINE__);
#else
        lmmp_free(marker);
#endif
        marker = next;
    }
}

static inline void lmmp_chech_memory(size_t size, const char* file, int line) {
    // TODO: check memory
    char msg[256];
    snprintf(msg, sizeof(msg), "Memory allocation failed (size: %zu bytes)", size);
    lmmp_abort(LAMMP_MEMORY_ALLOC_FAILURE, msg, file, line);
}

#if ALLOC_FREE_COUNT == 1
static int alloc_count = 0;
int lmmp_alloc_count(int cnt) {
    int new_cnt = cnt >= 0 ? cnt : alloc_count;
    cnt = alloc_count;
    alloc_count = new_cnt;
    return cnt;
}
#endif

#if MEMORY_CHECK == 1
void* lmmp_alloc(size_t size, const char* file, int line) { 
    if (size) {
        void* ret = lmmp_malloc_debug(size, file, line);
        if (ret == NULL)
            lmmp_chech_memory(size, file, line);
        #if ALLOC_FREE_COUNT == 1
        alloc_count++;
        #endif
        return ret;
    }
    return NULL;
}
#else
void* lmmp_alloc(size_t size) {
    if (size) {
        void* ret = malloc(size);
        if (ret == NULL) 
            lmmp_chech_memory(size, __FILE__, __LINE__);
        #if ALLOC_FREE_COUNT == 1
        alloc_count++;
        #endif
        return ret;
    }
    return NULL;
}
#endif

#if MEMORY_CHECK == 1
void* lmmp_realloc(void* oldptr, size_t new_size, const char* file, int line) {
    void* ret = lmmp_realloc_debug(oldptr, new_size, file, line);
    if (ret == NULL)
        lmmp_chech_memory(new_size, file, line);
    #if ALLOC_FREE_COUNT == 1
    alloc_count++;
    #endif
    return ret;
}
#else
void* lmmp_realloc(void* oldptr, size_t new_size) {
    void* ret = realloc(oldptr, new_size);
    if (ret == NULL) 
        lmmp_chech_memory(new_size, __FILE__, __LINE__);
    #if ALLOC_FREE_COUNT == 1
    alloc_count++;
    #endif
    return ret;
}
#endif

#if MEMORY_CHECK == 1
void lmmp_free(void* ptr, const char* file, int line) {
    if (ptr) {
        lmmp_free_debug(ptr, file, line);
        #if ALLOC_FREE_COUNT == 1
        alloc_count--;
        #endif
    }
}
#else
void lmmp_free(void* ptr) { 
    if (ptr) {
        free(ptr);
        #if ALLOC_FREE_COUNT == 1
        alloc_count--;
        #endif
    } 
}
#endif
