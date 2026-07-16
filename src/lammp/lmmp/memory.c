/**
 *  Copyright (C) 2026 HJimmyK(Jericho Knox)
 *
 *  This file is part of LAMMP.
 *
 *  LAMMP is free software: you can redistribute it and/or modify it under
 *  the terms of the GNU Lesser General Public License (LGPL) as published
 *   by the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed WITHOUT ANY WARRANTY.
 *
 *  See <https://www.gnu.org/licenses/>.
 */

#include "../../../include/lammp/impl/mparam.h"
#include "../../../include/lammp/impl/prime_table.h"
#include "../../../include/lammp/impl/tmp_alloc.h"
#include "../../../include/lammp/lmmpn.h"

#undef lmmp_alloc
#undef lmmp_realloc
#undef lmmp_free
#undef lmmp_stack_alloc
#undef lmmp_stack_free
#undef lmmp_leak_tracker
#define HSIZE sizeof(void*)

LAMMP_THREAD_LOCAL lmmp_heap_allocator_t global_heap = {
    .alloc   = (lmmp_heap_alloc_fn)malloc,
    .free    = (lmmp_heap_free_fn)free,
    .realloc = (lmmp_realloc_fn)realloc,
};

#define heap_alloc_func global_heap.alloc
#define heap_free_func  global_heap.free
#define realloc_func    global_heap.realloc

static LAMMP_THREAD_LOCAL int heap_alloc_count = 0;

LAMMP_THREAD_LOCAL lmmp_memory_ctx lmmp_tmpmem_ctx = {
    .stack_begin = NULL,
    .stack_end   = NULL,
    .stack_top   = NULL,
    .pool_begin  = NULL,
    .pool_top    = NULL,
    .remain      = 0,
    .capacity    = 0,
};

int lmmp_stack_deinit(void) {
    if (lmmp_tmpmem_ctx.stack_begin != NULL) {
        heap_free_func(lmmp_tmpmem_ctx.stack_begin);
        if (lmmp_tmpmem_ctx.pool_begin != NULL)
            heap_free_func(lmmp_tmpmem_ctx.pool_begin);
        lmmp_tmpmem_ctx.stack_begin = NULL;
        lmmp_tmpmem_ctx.stack_end = NULL;
        lmmp_tmpmem_ctx.stack_top = NULL;
        lmmp_tmpmem_ctx.pool_begin = NULL;
        lmmp_tmpmem_ctx.pool_top = NULL;
        lmmp_tmpmem_ctx.remain = 0;   // Reset remain to 0
        lmmp_tmpmem_ctx.capacity = 0; // Reset capacity to 0
        return 0;
    } else {
        return -1;
    }
}

int lmmp_stack_init(size_t size) {
    if (lmmp_tmpmem_ctx.stack_begin != NULL) {
        return -1;
    } else {
        lmmp_tmpmem_ctx.stack_begin = heap_alloc_func(LAMMP_DEFAULT_STACK_SIZE);
        if (lmmp_tmpmem_ctx.stack_begin == NULL) {
            lmmp_abort(LAMMP_ERROR_MEMORY_ALLOC_FAILURE, "Failed to allocate stack memory", __func__, __LINE__);
        }
        lmmp_tmpmem_ctx.stack_end = (mp_byte_t*)(lmmp_tmpmem_ctx.stack_begin) + LAMMP_DEFAULT_STACK_SIZE;
        lmmp_tmpmem_ctx.stack_top = lmmp_tmpmem_ctx.stack_begin;
        if (size > LAMMP_MAX_ALIGN) {
            // 只有当size大于最大对齐单位时，才分配缓冲池
            lmmp_tmpmem_ctx.pool_begin = heap_alloc_func(size);
            if (lmmp_tmpmem_ctx.pool_begin == NULL) {
                heap_free_func(lmmp_tmpmem_ctx.stack_begin);  // Free old stack memory
                lmmp_abort(LAMMP_ERROR_MEMORY_ALLOC_FAILURE, "Failed to allocate pool memory", __func__, __LINE__);
            }
            lmmp_tmpmem_ctx.pool_top = lmmp_tmpmem_ctx.pool_begin;
            lmmp_tmpmem_ctx.remain = size;
            lmmp_tmpmem_ctx.capacity = size;
        }
        return 0;
    }
}

void lmmp_set_heap_allocator(const lmmp_heap_allocator_t* heap) {
    if (heap == NULL)
        return;
    lmmp_global_deinit();
#if LAMMP_DEBUG_MEMORY_LEAK == 1
    lmmp_leak_tracker(__func__, __LINE__); // Check for memory leaks before setting new allocator
#endif
    global_heap = *heap;
    lmmp_global_init();
}

#include "../../../include/lammp/impl/safe_memory.h"

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
    char msg[360] = {0};
    int offset = 0;
    const int max_len = sizeof(msg) - 1;
    bool t = false;
    if (heap_alloc_count != 0) {
        offset +=
            snprintf(msg + offset, max_len - offset, "Heap allocations not freed: %d block(s);\n", heap_alloc_count);
        t = true;
    }
    if (lmmp_tmpmem_ctx.stack_top != lmmp_tmpmem_ctx.stack_begin) {
        offset += snprintf(msg + offset, max_len - offset,
                           "Default stack allocator is not empty. top: %p, begin: %p, end: %p;\n",
                           lmmp_tmpmem_ctx.stack_top, lmmp_tmpmem_ctx.stack_begin, lmmp_tmpmem_ctx.stack_end);
        t = true;
    }
    if (lmmp_tmpmem_ctx.pool_top != lmmp_tmpmem_ctx.pool_begin) {
        offset += snprintf(msg + offset, max_len - offset,
                           "Default pool allocator is not empty. top: %p, begin: %p; remain: %zu bytes, capacity: %zu "
                           "bytes;\n",
                           lmmp_tmpmem_ctx.pool_top, lmmp_tmpmem_ctx.pool_begin, lmmp_tmpmem_ctx.remain,
                           lmmp_tmpmem_ctx.capacity);
        t = true;
    }
    if (t) {
        lmmp_abort(LAMMP_ERROR_MEMORY_LEAK, msg, func, line);
    }
}

#if LAMMP_DEBUG_MEMORY_CHECK == 1
void* lmmp_alloc(size_t size, const char* func, int line) {
    void* ret = lmmp_alloc_debug(size, func, line);
#if LAMMP_DEBUG_MEMORY_LEAK == 1
    heap_alloc_count++;
#endif
    return ret;
}
#else
static inline void lmmp_memory_abort(size_t size, const char* func, int line) {
    char msg[64];
    snprintf(msg, sizeof(msg), "Memory allocation failed (size: %zu bytes)", size);
    lmmp_abort(LAMMP_ERROR_MEMORY_ALLOC_FAILURE, msg, func, line);
}

void* lmmp_alloc(size_t size) {
#if LAMMP_DEBUG_PARAM_ASSERT_CHECK == 1
    if (size == 0) {
        lmmp_abort(LAMMP_ERROR_MEMORY_ALLOC_FAILURE, "Allocating zero bytes is not allowed.", func, line);
        return NULL;
    }
#endif  // LAMMP_DEBUG_PARAM_ASSERT_CHECK == 1
    void* ret = heap_alloc_func(size);
    if (ret == NULL)
        lmmp_memory_abort(size, __func__, __LINE__);
#if LAMMP_DEBUG_MEMORY_LEAK == 1
    heap_alloc_count++;
#endif
    return ret;
}
#endif

#if LAMMP_DEBUG_MEMORY_CHECK == 1
void* lmmp_realloc(void* oldptr, size_t new_size, const char* func, int line) {
    void* ret = lmmp_realloc_debug(oldptr, new_size, func, line);
    return ret;
}
#else
void* lmmp_realloc(void* oldptr, size_t new_size) {
#if LAMMP_DEBUG_PARAM_ASSERT_CHECK == 1
    if (new_size == 0) {
        lmmp_abort(LAMMP_ERROR_MEMORY_ALLOC_FAILURE, "Reallocating zero bytes is not allowed.", func, line);
        return NULL;
    }
#endif // LAMMP_DEBUG_PARAM_ASSERT_CHECK == 1
    void* ret = realloc_func(oldptr, new_size);
    if (ret == NULL) {
        lmmp_memory_abort(new_size, __func__, __LINE__);
    }
    return ret;
}
#endif // LAMMP_DEBUG_MEMORY_CHECK == 1

#if LAMMP_DEBUG_MEMORY_CHECK == 1
void lmmp_free(void* ptr, const char* func, int line) {
    lmmp_free_debug(ptr, func, line);
#if LAMMP_DEBUG_MEMORY_LEAK == 1
    heap_alloc_count--;
#endif
}
#else
void lmmp_free(void* ptr) {
    heap_free_func(ptr);
#if LAMMP_DEBUG_MEMORY_LEAK == 1
    heap_alloc_count--;
#endif
}
#endif

void lmmp_global_init(void) {
    lmmp_stack_init(LAMMP_POOL_SIZE);
}

void lmmp_global_deinit(void) {
    lmmp_stack_deinit();
    lmmp_prime_int_table_free_();
}