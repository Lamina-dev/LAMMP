/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../../include/lammp/impl/prime_table.h"
#include "../../include/lammp/impl/tmp_alloc.h"
#include "../../include/lammp/lmmpn.h"


#undef lmmp_alloc
#undef lmmp_realloc
#undef lmmp_free
#undef lmmp_stack_alloc
#undef lmmp_stack_free
#undef lmmp_leak_tracker
#define HSIZE sizeof(void*)

THREAD_LOCAL static lmmp_heap_alloctor_t global_heap = {
    malloc,
    free,
    realloc,
};

THREAD_LOCAL static int heap_alloc_count = 0;

#define heap_alloc_func global_heap.alloc
#define heap_free_func global_heap.free
#define realloc_func global_heap.realloc

THREAD_LOCAL void* lmmp_stack_begin = NULL;
THREAD_LOCAL void* lmmp_stack_end = NULL;
THREAD_LOCAL void* lmmp_stack_top = NULL;

#define stack_get_top_func() (lmmp_stack_top)
#define stack_set_top_func(top) (lmmp_stack_top = top)

void lmmp_stack_reset(size_t size) {
    if (size) {
        if (!((mp_byte_t*)lmmp_stack_begin + size < (mp_byte_t*)lmmp_stack_end)) {
            lmmp_stack_begin = realloc_func(lmmp_stack_begin, size);
            lmmp_stack_end = (mp_byte_t*)lmmp_stack_begin + size;
        }
        if (lmmp_stack_begin == NULL) {
            lmmp_abort(LAMMP_ERROR_MEMORY_ALLOC_FAILURE, "Default stack allocation failed", __func__, __LINE__);
        }
        lmmp_stack_top = lmmp_stack_begin;
    } else {
#if LAMMP_DEBUG_MEMORY_LEAK == 1
        if (lmmp_stack_top != lmmp_stack_begin) {
            char msg[128];
            snprintf(msg, sizeof(msg), "Default stack allocator is not empty. top: %p, begin: %p, end: %p\n",
                     lmmp_stack_top, lmmp_stack_begin, lmmp_stack_end);
            lmmp_abort(LAMMP_ERROR_MEMORY_LEAK, msg, __func__, __LINE__);
        }
#endif
        if (lmmp_stack_begin)
            heap_free_func(lmmp_stack_begin);
        lmmp_stack_begin = NULL;
        lmmp_stack_end = NULL;
        lmmp_stack_top = NULL;
    }
}

void lmmp_stack_init(void) {
    if (lmmp_stack_begin != NULL) {
        return;
    } else {
        lmmp_stack_begin = heap_alloc_func(LAMMP_DEFAULT_STACK_SIZE);
        lmmp_stack_end = (mp_byte_t*)lmmp_stack_begin + LAMMP_DEFAULT_STACK_SIZE;
        lmmp_stack_top = lmmp_stack_begin;
    }
}

void lmmp_set_heap_alloctor(const lmmp_heap_alloctor_t* heap) {
    if (heap == NULL)
        return;
    lmmp_stack_reset(0);
#if LAMMP_DEBUG_MEMORY_LEAK == 1
    if (heap_alloc_count != 0) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Older heap allocations not freed: %d block(s)", heap_alloc_count);
        lmmp_abort(LAMMP_ERROR_MEMORY_LEAK, msg, __func__, __LINE__);
    }
#endif
    global_heap = *heap;
}

void* lmmp_temp_heap_alloc_(void** pmarker, size_t size) {
    /*
     * pmarker is a head pointer to a linked list of allocated memory blocks.
     * Each allocated block has a header of size HSIZE, which is used to store the
     * next pointer of the block. The actual data starts at (mp_byte_t*)p + offset.
     */
    const size_t offset = LMMP_ROUND_UP_MULTIPLE(HSIZE, LAMMP_MAX_ALIGN);
    void* p = heap_alloc_func(size + offset);
    *(void**)p = *pmarker;
    *pmarker = p;
    return (mp_byte_t*)p + offset;
}

void lmmp_temp_heap_free_(void* marker) {
    /*
     *  Free all allocated memory blocks in the linked list pointed to by pmarker.
     */
    while (marker) {
        void* next = *(void**)marker;
        heap_free_func(marker);
        marker = next;
    }
}

#include "../../include/lammp/impl/safe_memory.h"

static inline void lmmp_chech_memory(size_t size, const char* func, int line) {
    char msg[64];
    snprintf(msg, sizeof(msg), "Memory allocation failed (size: %zu bytes)", size);
    lmmp_abort(LAMMP_ERROR_MEMORY_ALLOC_FAILURE, msg, func, line);
}

int lmmp_alloc_count(int cnt) {
    if (cnt != 0) {
        int new_cnt = cnt;
        cnt = heap_alloc_count;
        heap_alloc_count = new_cnt;
        return cnt;
    }
    return heap_alloc_count;
}

void lmmp_leak_tracker(const char* func, int line) {
    char msg[192] = {0};
    int offset = 0;
    const int max_len = sizeof(msg) - 1;
    int t = 0;
    if (heap_alloc_count != 0) {
        offset +=
            snprintf(msg + offset, max_len - offset, "Heap allocations not freed: %d block(s)\n", heap_alloc_count);
        t = 1;
    }
    if (t) {
        lmmp_abort(LAMMP_ERROR_MEMORY_LEAK, msg, func, line);
    }
}

#if LAMMP_DEBUG_MEMORY_CHECK == 1
void* lmmp_alloc(size_t size, const char* func, int line) {
    if (size) {
        void* ret = lmmp_alloc_debug(size, func, line);
#if LAMMP_DEBUG_MEMORY_LEAK == 1
        heap_alloc_count++;
#endif
        return ret;
    }
    return NULL;
}
#else
void* lmmp_alloc(size_t size) {
    if (size) {
        void* ret = heap_alloc_func(size);
        if (ret == NULL)
            lmmp_chech_memory(size, __func__, __LINE__);
#if LAMMP_DEBUG_MEMORY_LEAK == 1
        heap_alloc_count++;
#endif
        return ret;
    }
    return NULL;
}
#endif

#if LAMMP_DEBUG_MEMORY_CHECK == 1
void* lmmp_realloc(void* oldptr, size_t new_size, const char* func, int line) {
    void* ret = lmmp_realloc_debug(oldptr, new_size, func, line);
    return ret;
}
#else
void* lmmp_realloc(void* oldptr, size_t new_size) {
    void* ret = realloc_func(oldptr, new_size);
    if (ret == NULL)
        lmmp_chech_memory(new_size, __func__, __LINE__);
    return ret;
}
#endif

#if LAMMP_DEBUG_MEMORY_CHECK == 1
void lmmp_free(void* ptr, const char* func, int line) {
    if (ptr) {
        lmmp_free_debug(ptr, func, line);
#if LAMMP_DEBUG_MEMORY_LEAK == 1
        heap_alloc_count--;
#endif
    }
}
#else
void lmmp_free(void* ptr) {
    if (ptr) {
        heap_free_func(ptr);
#if LAMMP_DEBUG_MEMORY_LEAK == 1
        heap_alloc_count--;
#endif
    }
}
#endif

#if LAMMP_DEBUG_MEMORY_CHECK != 1

#define SIZE_SIZE LMMP_ROUND_UP_MULTIPLE(sizeof(size_t), LAMMP_MAX_ALIGN)

void* lmmp_stack_alloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    size_t total_size = SIZE_SIZE + LMMP_ROUND_UP_MULTIPLE(size, LAMMP_MAX_ALIGN);
    void* old_top = stack_get_top_func();
    void* new_top = (mp_byte_t*)old_top + total_size;
#if LAMMP_DEBUG_STACK_OVERFLOW_CHECK == 1
    if (new_top > global_stack.end) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Stack overflow (trying to allocate: %zu bytes, stack remaining: %zu bytes)",
                 total_size, (size_t)((mp_byte_t*)global_stack.end - (mp_byte_t*)old_top));
        lmmp_abort(LAMMP_ERROR_MEMORY_ALLOC_FAILURE, msg, __func__, __LINE__);
    }
#endif  // LAMMP_DEBUG_STACK_OVERFLOW_CHECK == 1
    stack_set_top_func(new_top);
    *(size_t*)old_top = total_size;
    return (mp_byte_t*)old_top + SIZE_SIZE;
}

void lmmp_stack_free(void* ptr) {
    if (ptr == NULL) {
        return;
    }
#if LAMMP_DEBUG_STACK_OVERFLOW_CHECK == 1
    if (ptr < global_stack.begin || ptr >= global_stack.end) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Invalid stack pointer (trying to free: %p ; stack start: %p , stack end: %p )", ptr,
                 global_stack.begin, global_stack.end);
        lmmp_abort(LAMMP_ERROR_MEMORY_FREE_FAILURE, msg, __func__, __LINE__);
    }
#endif  // LAMMP_DEBUG_STACK_OVERFLOW_CHECK == 1
    void* old_top = stack_get_top_func();
    size_t total_size = *(size_t*)((mp_byte_t*)ptr - SIZE_SIZE);
    void* new_top = (mp_byte_t*)old_top - total_size;
#if LAMMP_DEBUG_STACK_OVERFLOW_CHECK == 1
    if (new_top < global_stack.begin || new_top > global_stack.end) {
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "Stack underflow (trying to free: %p , size: %zu bytes ; stack start: %p , stack end: %p ) \n%s", ptr,
                 total_size - SIZE_SIZE, global_stack.begin, global_stack.end,
                 "Likely cause: Previous stack buffer overflow corrupted the memory header.");
        lmmp_abort(LAMMP_ERROR_MEMORY_FREE_FAILURE, msg, __func__, __LINE__);
    }
#endif  // LAMMP_DEBUG_STACK_OVERFLOW_CHECK == 1
    stack_set_top_func(new_top);
}

#else

typedef struct {
    size_t total_size;         // 总分配大小（原有）
    size_t extra_size;         // 额外分配的魔数区域大小
    size_t magic_addr_offset;  // 魔数起始地址相对于old_top的偏移
    void* last_ptr;            // 上一次分配的指针
    const char* func;          // 分配时的函数名
    int line;                  // 分配时的文件行号
} StackHeader;

#define HEADER_SIZE sizeof(StackHeader)

#define MAGIC_NUMBER 0xDEADBEEF
#define MAGIC_SIZE sizeof(unsigned int)

THREAD_LOCAL static void* global_stack_last_ptr = NULL;  // 最后一次分配的指针

void* lmmp_stack_alloc(size_t size, const char* func, int line) {
    if (size == 0) {
        return NULL;
    }

    size_t base_data_size = LMMP_ROUND_UP_MULTIPLE(size, LAMMP_MAX_ALIGN);
    size_t base_total_size = HEADER_SIZE + base_data_size;

    size_t extra_size = (base_total_size * LAMMP_MEMORY_MORE_ALLOC_TIMES) / 10;
    if (extra_size < MAGIC_SIZE) {
        extra_size = MAGIC_SIZE;
    }
    size_t total_size = base_total_size + extra_size;

    void* old_top = stack_get_top_func();
    void* new_top = (mp_byte_t*)old_top + total_size;
    if (new_top > lmmp_stack_end) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Stack overflow (alloc: %zu bytes, remaining: %zu bytes)", total_size,
                 (size_t)((mp_byte_t*)lmmp_stack_end - (mp_byte_t*)old_top));
        lmmp_abort(LAMMP_ERROR_MEMORY_ALLOC_FAILURE, msg, func, line);
    }

    StackHeader* header = (StackHeader*)old_top;
    header->total_size = total_size;
    header->extra_size = extra_size;
    header->magic_addr_offset = base_total_size;  // 魔数起始地址 = old_top + base_total_size
    header->last_ptr = global_stack_last_ptr;
    header->func = func;
    header->line = line;
    void* magic_addr = (mp_byte_t*)old_top + header->magic_addr_offset;
    memset(magic_addr, 0, extra_size);
    for (size_t i = 0; i + MAGIC_SIZE <= extra_size; i += MAGIC_SIZE) {
        *(unsigned int*)((mp_byte_t*)magic_addr + i) = MAGIC_NUMBER;
    }

    stack_set_top_func(new_top);
    void* alloc_ptr = (mp_byte_t*)old_top + HEADER_SIZE;

    global_stack_last_ptr = alloc_ptr;

    return alloc_ptr;
}

void lmmp_stack_free(void* ptr, const char* func, int line) {
    if (ptr == NULL) {
        return;
    }
    if (ptr < lmmp_stack_begin || ptr >= lmmp_stack_end) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Invalid stack pointer, trying to free %p (stack start: %p, end: %p)", ptr,
                 lmmp_stack_begin, lmmp_stack_end);
        lmmp_abort(LAMMP_ERROR_MEMORY_FREE_FAILURE, msg, func, line);
    }
    if (ptr != global_stack_last_ptr) {
        char msg[96];
        snprintf(msg, sizeof(msg), "Invalid stack pointer. Expected %p, but try to free %p", global_stack_last_ptr,
                 ptr);
        lmmp_abort(LAMMP_ERROR_MEMORY_FREE_FAILURE, msg, func, line);
    }

    StackHeader* header = (StackHeader*)((mp_byte_t*)ptr - HEADER_SIZE);
    size_t total_size = header->total_size;
    size_t extra_size = header->extra_size;
    void* magic_addr = (mp_byte_t*)header + header->magic_addr_offset;
    global_stack_last_ptr = header->last_ptr;
    int magic_corrupted = 0;
    for (size_t i = 0; i + MAGIC_SIZE <= extra_size; i += MAGIC_SIZE) {
        unsigned int magic = *(unsigned int*)((mp_byte_t*)magic_addr + i);
        if (magic != MAGIC_NUMBER) {
            magic_corrupted = 1;
            break;
        }
    }
    if (magic_corrupted) {
        char error_buf[512];
        int offset = 0;
        const int buf_size = sizeof(error_buf);

#define SAFE_APPEND(fmt, ...)                                                                      \
    do {                                                                                           \
        if (offset < buf_size) {                                                                   \
            int n = snprintf(error_buf + offset, (size_t)(buf_size - offset), fmt, ##__VA_ARGS__); \
            if (n > 0)                                                                             \
                offset += n;                                                                       \
        }                                                                                          \
    } while (0)

        SAFE_APPEND("Stack buffer overflow detected! Magic number corrupted at %p (ptr: %p, size: %zu)\n", magic_addr,
                    ptr, (mp_byte_t*)magic_addr - (mp_byte_t*)ptr);
        SAFE_APPEND("Stack buffer header:%s", "\n");
        SAFE_APPEND("  allocated at: [%s]:%d\n", header->func, header->line);
        SAFE_APPEND("  total size:   %zu bytes\n", header->total_size);
        SAFE_APPEND("  extra size:   %zu bytes (%.0f%% of user size)\n", header->extra_size,
                    LAMMP_MEMORY_MORE_ALLOC_TIMES * 10.0);
        SAFE_APPEND("  magic offset: %zu\n", header->magic_addr_offset);
        SAFE_APPEND("  last ptr:     %p\n", header->last_ptr);
        lmmp_abort(LAMMP_ERROR_OUT_OF_BOUNDS, error_buf, func, line);
    }

    void* old_top = stack_get_top_func();
    void* new_top = (mp_byte_t*)old_top - total_size;
    if (new_top < lmmp_stack_begin || new_top > lmmp_stack_end) {
        char msg[192];
        snprintf(msg, sizeof(msg), "Stack underflow (free: %p, size: %zu; stack start: %p, end: %p)", ptr,
                 (mp_byte_t*)magic_addr - (mp_byte_t*)ptr, lmmp_stack_begin, lmmp_stack_end);
        lmmp_abort(LAMMP_ERROR_MEMORY_FREE_FAILURE, msg, func, line);
    }

    if (new_top != lmmp_stack_begin) {
        if ((mp_byte_t*)new_top + HEADER_SIZE > (mp_byte_t*)lmmp_stack_end) {
            lmmp_abort(LAMMP_ERROR_MEMORY_FREE_FAILURE, "Invalid previous block header", func, line);
        }
    }
    stack_set_top_func(new_top);
}
#endif  // LAMMP_DEBUG_MEMORY_CHECK != 1

void lmmp_global_deinit(void) {
    lmmp_stack_reset(0);
    lmmp_prime_int_table_free_();
}