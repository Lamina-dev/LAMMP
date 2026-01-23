#ifndef SAFE_MALLOC_H
#define SAFE_MALLOC_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 内存块头部
typedef struct {
    size_t user_size;   // 用户请求大小
    size_t total_size;  // 总分配大小
    uint32_t magic;     // 魔数
    const char* file;   // 分配位置文件
    int line;           // 分配位置行号
} MemHeader;

#define MEM_MAGIC 0xDEADBEEF

// 前后哨兵大小
#define GUARD_SIZE 16

// 哨兵填充模式
static const char FRONT_GUARD_PATTERN[GUARD_SIZE] = "FONT_GUARD_1234";
static const char REAR_GUARD_PATTERN[GUARD_SIZE] = "REAR_GUARD_4321";

// 对齐到8字节边界
#define ALIGNMENT 8
static inline size_t align_up(size_t size) { return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1); }

// 调试输出函数
static inline void debug_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

/**
 * 检查是否发生尾部溢出（只检查后哨兵）
 */
static inline int check_rear_guard_overflow(MemHeader* hdr, void* user_ptr) {
    if (hdr == NULL || user_ptr == NULL)
        return 0;

    // 计算后哨兵位置
    size_t aligned_user_size = align_up(hdr->user_size);
    char* rear_guard = (char*)user_ptr + aligned_user_size;

    // 检查后哨兵是否被修改
    if (memcmp(rear_guard, REAR_GUARD_PATTERN, GUARD_SIZE) != 0) {
        // 输出详细的调试信息
        debug_printf("========== TAIL OVERFLOW DETECTED ==========\n");
        debug_printf("Memory allocated at: %s:%d\n", hdr->file, hdr->line);
        debug_printf("User requested size: %zu bytes\n", hdr->user_size);
        debug_printf("User pointer: %p\n", user_ptr);
        debug_printf("Rear guard position: %p\n", rear_guard);

        // 显示后哨兵被修改的字节
        char* rear_data = (char*)rear_guard;
        debug_printf("Rear guard corruption details:\n");

        int first_corruption = -1;
        int corruption_count = 0;

        for (int i = 0; i < GUARD_SIZE; i++) {
            if (rear_data[i] != REAR_GUARD_PATTERN[i]) {
                if (first_corruption == -1)
                    first_corruption = i;
                corruption_count++;

                debug_printf(
                    "  Offset %2d: Expected 0x%02X('%c'), Got 0x%02X('%c')\n", i, (unsigned char)REAR_GUARD_PATTERN[i],
                    (REAR_GUARD_PATTERN[i] >= 32 && REAR_GUARD_PATTERN[i] < 127) ? REAR_GUARD_PATTERN[i] : '.',
                    (unsigned char)rear_data[i], (rear_data[i] >= 32 && rear_data[i] < 127) ? rear_data[i] : '.');
            }
        }

        debug_printf("Total corrupted bytes: %d\n", corruption_count);

        // 如果只有第一个字节被修改，可能是字符串没有终止符导致的溢出
        if (corruption_count == 1 && first_corruption == 0) {
            debug_printf("Likely cause: Missing null terminator in string\n");
        }
        // 如果多个连续字节被修改，可能是数组越界
        else if (corruption_count > 1 && first_corruption == 0) {
            debug_printf("Likely cause: Array/buffer overflow\n");
        }

        debug_printf("===============================================\n");

        return 1;
    }

    return 0;
}

/**
 * 调试版内存分配
 */
static inline void* my_malloc_debug(size_t size, const char* file, int line) {
    if (size == 0)
        return NULL;

    // 计算总分配大小
    size_t header_size = align_up(sizeof(MemHeader));
    size_t aligned_user_size = align_up(size);

    // 总大小 = 头部 + 前哨兵 + 用户内存 + 后哨兵
    size_t total_size = header_size + GUARD_SIZE + aligned_user_size + GUARD_SIZE;

    // 分配内存
    void* base = malloc(total_size);
    if (base == NULL) {
        debug_printf("Memory allocation failed: requested %zu bytes\n", total_size);
        return NULL;
    }

    // 清零整个内存块
    memset(base, 0, total_size);

    // 设置内存布局
    MemHeader* hdr = (MemHeader*)base;
    void* front_guard = (char*)base + header_size;
    void* user_ptr = (char*)front_guard + GUARD_SIZE;
    void* rear_guard = (char*)user_ptr + aligned_user_size;

    hdr->user_size = size;
    hdr->total_size = total_size;
    hdr->file = file;
    hdr->line = line;
    hdr->magic = MEM_MAGIC;

    // 设置前后哨兵
    memcpy(front_guard, FRONT_GUARD_PATTERN, GUARD_SIZE);
    memcpy(rear_guard, REAR_GUARD_PATTERN, GUARD_SIZE);

    // 在用户内存区域填充特定模式（0xCD表示未初始化内存）
    memset(user_ptr, 0xCD, aligned_user_size);

    return user_ptr;
}

/**
 * 调试版内存释放
 */
static inline void my_free_debug(void* ptr, const char* file, int line) {
    if (ptr == NULL) {
        debug_printf("Attempt to free NULL pointer at %s:%d\n", file, line);
        return;
    }

    // 找回头部
    size_t header_size = align_up(sizeof(MemHeader));
    MemHeader* hdr = (MemHeader*)((char*)ptr - header_size - GUARD_SIZE);

    if (hdr->magic != MEM_MAGIC) {
        debug_printf("ERROR: Invalid free at %s:%d - bad magic number (got 0x%08X, expected 0x%08X)\n", file, line,
                     hdr->magic, MEM_MAGIC);
        return;
    }

    // 检查尾部溢出
    int overflow_detected = check_rear_guard_overflow(hdr, ptr);

    if (overflow_detected) {
        debug_printf("WARNING: Tail overflow detected during free at %s:%d\n", file, line);
        debug_printf("  Memory was originally allocated at %s:%d\n", hdr->file, hdr->line);
    }

    // 释放前填充特定模式（0xDD表示已释放内存）
    memset(hdr, 0xDD, hdr->total_size);

    //debug_printf("Freed memory at %p (original user pointer: %p) from %s:%d\n", hdr, ptr, file, line);

    free(hdr);
}

/**
 * 调试版内存重新分配
 */
static inline void* my_realloc_debug(void* ptr, size_t new_size, const char* file, int line) {
    if (ptr == NULL) {
        return my_malloc_debug(new_size, file, line);
    }

    if (new_size == 0) {
        my_free_debug(ptr, file, line);
        return NULL;
    }

    size_t header_size = align_up(sizeof(MemHeader));
    MemHeader* old_hdr = (MemHeader*)((char*)ptr - header_size - GUARD_SIZE);

    if (old_hdr->magic != MEM_MAGIC) {
        debug_printf("ERROR: Invalid pointer in realloc at %s:%d\n", file, line);
        return NULL;
    }

    // 检查尾部溢出
    check_rear_guard_overflow(old_hdr, ptr);

    void* new_ptr = my_malloc_debug(new_size, file, line);
    if (new_ptr == NULL) {
        return NULL;
    }

    size_t copy_size = (old_hdr->user_size < new_size) ? old_hdr->user_size : new_size;
    memcpy(new_ptr, ptr, copy_size);

    // 释放旧内存块
    my_free_debug(ptr, file, line);

    return new_ptr;
}

/**
 * 检查内存溢出
 */
static inline int check_memory_overflow(void* ptr) {
    if (ptr == NULL)
        return 0;

    size_t header_size = align_up(sizeof(MemHeader));
    MemHeader* hdr = (MemHeader*)((char*)ptr - header_size - GUARD_SIZE);

    if (hdr->magic != MEM_MAGIC) {
        debug_printf("ERROR: Invalid pointer in check_memory_overflow\n");
        return 0;
    }

    return check_rear_guard_overflow(hdr, ptr);
}

#define CHECK_OVERFLOW(ptr) check_memory_overflow(ptr)

#endif  // SAFE_MALLOC_H