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

#ifndef __LAMMP_TMP_ALLOC_H__
#define __LAMMP_TMP_ALLOC_H__

#include "../lmmp.h"
#include <stdio.h>

extern void* lmmp_stack_begin;
extern void* lmmp_stack_end;
extern void* lmmp_stack_top;

/**
 * @brief 临时堆内存分配函数
 * @param pmarker 标记
 * @param size 要分配的内存字节数
 */
void* lmmp_temp_heap_alloc_(void** pmarker, size_t size);

/**
 * @brief 临时堆内存释放函数
 * @param marker 要释放的临时内存标记
 */
void lmmp_temp_heap_free_(void* marker);

static inline void* lmmp_temp_stack_alloc_(void** pmarker, size_t size) {
    /*
     * On the first call, *pmarker is a null pointer.
     * We will use *pmarker to record the stack frame at this time.
     * When allocating memory subsequently, we will not modify *pmarker.
     * Until all stack memory is finally released, we will move to the initial stack position at once,
     * which is the position recorded by *pmarker.
     */
    void* p = lmmp_stack_top;
    size_t offset = LMMP_ROUND_UP_MULTIPLE(size, LAMMP_MAX_ALIGN);
    void* new_top = (void*)((mp_byte_t*)p + offset);
#if LAMMP_DEBUG_STACK_OVERFLOW_CHECK == 1
    if (new_top > lmmp_stack_end) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Stack overflow (trying to allocate: %zu bytes, stack remaining: %zu bytes)", offset,
                 (size_t)((mp_byte_t*)lmmp_stack_end - (mp_byte_t*)lmmp_stack_top));
        lmmp_abort(LAMMP_ERROR_MEMORY_ALLOC_FAILURE, msg, __func__, __LINE__);
    }
#endif
    lmmp_stack_top = new_top;
    if (*pmarker == NULL)
        *pmarker = p;
    return p;
}

static inline void lmmp_temp_stack_free_(void* marker) { 
    lmmp_stack_top = marker;
}

// 临时内存标记声明：用于跟踪临时内存分配
#define TEMP_DECL                                                           \
    void *lmmp_temp_alloc_marker_ = NULL, *lmmp_temp_stack_marker_ = NULL 

#define TEMP_B_DECL void* lmmp_temp_alloc_marker_ = NULL
#define TEMP_S_DECL void* lmmp_temp_stack_marker_ = NULL

#define TEMP_SALLOC_THRESHOLD 0x7f00  // 小内存分配阈值（小于等于该值的内存分配在栈上）

// 栈内存分配：使用lmmp_temp_stack_alloc_在栈上分配n字节内存（小内存）
#define TEMP_SALLOC(n) lmmp_temp_stack_alloc_(&lmmp_temp_stack_marker_, (n))
// 堆内存分配：使用lmmp_temp_heap_alloc_在堆上分配n字节内存（大内存）
#define TEMP_BALLOC(n) lmmp_temp_heap_alloc_(&lmmp_temp_alloc_marker_, (n))
// 临时内存分配：小内存用栈，大内存用堆
#define TEMP_TALLOC(n) ((n) <= TEMP_SALLOC_THRESHOLD ? TEMP_SALLOC(n) : TEMP_BALLOC(n))
// 类型化栈内存分配：分配n个type类型的栈内存
#define SALLOC_TYPE(n, type) ((type*)TEMP_SALLOC((n) * sizeof(type)))
// 类型化堆内存分配：分配n个type类型的堆内存
#define BALLOC_TYPE(n, type) ((type*)TEMP_BALLOC((n) * sizeof(type)))
// 类型化临时内存分配：智能选择栈/堆分配n个type类型内存
#define TALLOC_TYPE(n, type) ((type*)TEMP_TALLOC((n) * sizeof(type)))
// 临时内存释放：释放所有通过TEMP_XALLOC系列函数分配的临时内存
#define TEMP_FREE                                           \
    do {                                                    \
        if (lmmp_temp_alloc_marker_)                        \
            lmmp_temp_heap_free_(lmmp_temp_alloc_marker_);  \
        if (lmmp_temp_stack_marker_)                        \
            lmmp_temp_stack_free_(lmmp_temp_stack_marker_); \
    } while (0)
#define TEMP_B_FREE                                        \
    do {                                                   \
        if (lmmp_temp_alloc_marker_)                       \
            lmmp_temp_heap_free_(lmmp_temp_alloc_marker_); \
    } while (0)
#define TEMP_S_FREE                                         \
    do {                                                    \
        if (lmmp_temp_stack_marker_)                        \
            lmmp_temp_stack_free_(lmmp_temp_stack_marker_); \
    } while (0)

// 类型化内存分配：分配n个type类型的内存（堆）
#define ALLOC_TYPE(n, type) ((type*)lmmp_alloc((size_t)(n) * sizeof(type)))
// 类型化内存重分配：将p指向的内存重分配为new_size个type类型
#define REALLOC_TYPE(p, new_size, type) ((type*)lmmp_realloc((p), (new_size) * sizeof(type)))

#endif /* __LAMMP_TMP_ALLOC_H__ */