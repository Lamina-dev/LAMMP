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

#ifndef __LAMMP_SAFE_MEMORY_H__
#define __LAMMP_SAFE_MEMORY_H__

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
    uint64_t magic;     // 魔数，用于验证有效性
    size_t user_size;   // 用户请求的大小
    size_t total_size;  // 总分配大小（含头部、用户内存、额外内存）
    size_t extra_size;  // 额外分配的内存大小
    const char* func;   // 分配发生的函数名
    int line;           // 分配发生的行号
    uint32_t guard;     // 尾哨兵
} mem_header;

#define MEM_MAGIC 0xDEADBEEFDEADBEEFULL    // deadbeef
#define MEM_GUARD 0xDEADBEEFUL             // deadbeef
#define EXTRA_MEM_PATTERN 0xAA  // 额外内存填充模式

#define ALIGNMENT LAMMP_MAX_ALIGN

static inline size_t align_up(size_t size) { 
    return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1); 
}

/**
 * @brief 找到指定模式的连续内存区域，并记录起始和结束位置。
 */
static inline void find_corruption_range(
    const char*   data,
    unsigned char pattern,
    size_t        len,
    int*          first,
    int*          last,
    int*          count
) {
    *first = -1;
    *last = -1;
    *count = 0;

    for (size_t i = 0; i < len; i++) {
        if ((unsigned char)data[i] != pattern) {
            (*count)++;
            if (*first == -1)
                *first = (int)i;
            *last = (int)i;
        }
    }
}

/**
 * @brief 检查额外内存区域是否被修改。
 */
static inline int check_extra_memory_overflow(mem_header* hdr, void* user_ptr, const char* check_func, int check_line) {
    if (!hdr || !user_ptr || hdr->extra_size == 0)
        return 0;

    size_t aligned_user_size = align_up(hdr->user_size);
    char* extra_start = (char*)user_ptr + aligned_user_size;

    int first, last, count;
    find_corruption_range(extra_start, EXTRA_MEM_PATTERN, hdr->extra_size, &first, &last, &count);

    if (count > 0) {
        char error_buf[640];
        int offset = 0;
        const int buf_size = sizeof(error_buf);

#define SAFE_APPEND(...)                                                                    \
    do {                                                                                    \
        if (offset < buf_size) {                                                            \
            int n = snprintf(error_buf + offset, (size_t)(buf_size - offset), __VA_ARGS__); \
            if (n > 0)                                                                      \
                offset += n;                                                                \
        }                                                                                   \
    } while (0)

        SAFE_APPEND("Memory overflow (extra memory corruption) detected!%s", "\n");
        SAFE_APPEND("Memory header:%s", "\n");
        SAFE_APPEND("  allocated at: [%s]:%d\n", hdr->func, hdr->line);
        SAFE_APPEND("  checked at:   [%s]:%d\n", check_func, check_line);
        SAFE_APPEND("  user size:    %zu bytes\n", hdr->user_size);
        SAFE_APPEND("  extra size:   %zu bytes (%.0f%% of user size)\n", hdr->extra_size,
                    LAMMP_MEMORY_MORE_ALLOC_TIMES * 10.0);
        SAFE_APPEND("  user ptr:     %p\n", user_ptr);
        SAFE_APPEND("  extra memory: %p to %p\n", (void*)extra_start, (void*)(extra_start + hdr->extra_size - 1));
        SAFE_APPEND("  corrupted range: offset %d to %d (total %d bytes)\n", first, last, count);
        SAFE_APPEND("Likely cause: Buffer overflow beyond the end of the memory.%s", "\n");

        error_buf[buf_size - 1] = '\0';
        lmmp_abort(LAMMP_ERROR_OUT_OF_BOUNDS, error_buf, check_func, check_line);
        return 1;
    }
    return 0;
}

/**
 * @brief 检查内存块的完整性（包括头尾和额外内存区域）。
 */
static inline int check_memory_block_integrity(mem_header* hdr, void* user_ptr, const char* check_func, int check_line) {
    if (!hdr || !user_ptr)
        return 0;

    if (hdr->magic != MEM_MAGIC || hdr->guard != MEM_GUARD) {
        char error_buf[240];
        snprintf(error_buf, sizeof(error_buf),
                 "Memory header corruption detected!\n"
                 "  Magic: 0x%016llx (expected 0x%016llx)\n"
                 "  Guard: 0x%08lx (expected 0x%08lx)\n"
                 "Possible overflow or underflow or invalid pointer.",
                 hdr->magic, MEM_MAGIC, (unsigned long)hdr->guard, MEM_GUARD);
        lmmp_abort(LAMMP_ERROR_MEMORY_FREE_FAILURE, error_buf, check_func, check_line);
        return 1;
    }

    return check_extra_memory_overflow(hdr, user_ptr, check_func, check_line);
}

/**
 * @brief 调试版 malloc
 * @param size 要分配的内存大小
 * @param func 分配内存的函数名
 * @param line 分配内存的行号
 * @return 分配的内存块的指针
 */
static inline void* lmmp_alloc_debug(size_t size, const char* func, int line) {
    if (size == 0)
        return NULL;

    size_t extra_size = (size * LAMMP_MEMORY_MORE_ALLOC_TIMES) / 10;
    extra_size = align_up(extra_size);

    size_t header_size = align_up(sizeof(mem_header));
    size_t aligned_user_size = align_up(size);
    size_t total_size = header_size + aligned_user_size + extra_size;

    void* base = heap_alloc_func(total_size);
    if (!base) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Memory allocation failed (size: %zu bytes, extra: %zu bytes)", size, extra_size);
        lmmp_abort(LAMMP_ERROR_MEMORY_ALLOC_FAILURE, msg, func, line);
        return NULL;
    }

    mem_header* hdr = (mem_header*)base;
    void* user_ptr = (char*)base + header_size;
    void* extra_mem = (char*)user_ptr + aligned_user_size;

    hdr->magic = MEM_MAGIC;
    hdr->user_size = size;
    hdr->total_size = total_size;
    hdr->extra_size = extra_size;
    hdr->func = func;
    hdr->line = line;
    hdr->guard = MEM_GUARD;

    memset(extra_mem, EXTRA_MEM_PATTERN, extra_size);

    return user_ptr;
}

/**
 * @brief 调试版 free
 * @param ptr 指向要释放的内存块的指针
 * @param func 分配内存的函数名
 * @param line 分配内存的行号
 */
static inline void lmmp_free_debug(void* ptr, const char* func, int line) {
    if (!ptr)
        return;

    size_t header_size = align_up(sizeof(mem_header));
    mem_header* hdr = (mem_header*)((char*)ptr - header_size);

    check_memory_block_integrity(hdr, ptr, func, line);

    heap_free_func(hdr);
}

/**
 * @brief 调试版 realloc
 * @param ptr 指向要重新分配的内存块的指针
 * @param new_size 新的内存大小
 * @param func 分配内存的函数名
 * @param line 分配内存的行号
 * @return 新分配的内存块的指针
 */
static inline void* lmmp_realloc_debug(void* ptr, size_t new_size, const char* func, int line) {
    if (!ptr)
        return lmmp_alloc_debug(new_size, func, line);
    if (new_size == 0) {
        lmmp_abort(LAMMP_ERROR_MEMORY_ALLOC_FAILURE, "Reallocating zero bytes is not allowed.", func, line);
    }

    size_t header_size = align_up(sizeof(mem_header));
    mem_header* old_hdr = (mem_header*)((char*)ptr - header_size);

    check_memory_block_integrity(old_hdr, ptr, func, line);

    void* new_ptr = lmmp_alloc_debug(new_size, func, line);
    if (!new_ptr)
        return NULL;

    size_t copy_size = (old_hdr->user_size < new_size) ? old_hdr->user_size : new_size;
    memcpy(new_ptr, ptr, copy_size);

    lmmp_free_debug(ptr, func, line);
    return new_ptr;
}

#undef SAFE_APPEND
#undef MEM_MAGIC
#undef MEM_GUARD
#undef ALIGNMENT
#undef EXTRA_MEM_PATTERN

#endif  // __LAMMP_SAFE_MEMORY_H__