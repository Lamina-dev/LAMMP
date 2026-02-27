#ifndef __LAMMP_SAFE_MEMORY_H__
#define __LAMMP_SAFE_MEMORY_H__

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// === 配置宏 ===
// 额外分配的内存比例控制，默认额外分配10% (MORE_ALLOC_TIMES/10)
#ifndef LAMMP_MEMORY_MORE_ALLOC_TIMES
#define MORE_ALLOC_TIMES 1
#else
#define MORE_ALLOC_TIMES LAMMP_MEMORY_MORE_ALLOC_TIMES
#endif

// === 内存块头部结构 ===
typedef struct {
    size_t user_size;   // 用户请求的大小
    size_t total_size;  // 总分配大小（含头部、用户内存、额外内存）
    size_t extra_size;  // 额外分配的内存大小
    uint32_t magic;     // 魔数，用于验证有效性
    const char* file;   // 分配发生的文件
    int line;           // 分配发生的行号
} MemHeader;

#define MEM_MAGIC 0xDEADBEEF
#define EXTRA_MEM_PATTERN 0xAA  // 额外内存填充模式（-86的补码即0xAA）

#define ALIGNMENT 8
static inline size_t align_up(size_t size) { return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1); }

// === 辅助函数：查找连续修改的范围 ===
static inline void find_corruption_range(const char* data,
                                         unsigned char pattern,
                                         size_t len,
                                         int* first,
                                         int* last,
                                         int* count) {
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

// === 检查额外内存区域（上溢检测）===
static inline int check_extra_memory_overflow(MemHeader* hdr, void* user_ptr, const char* check_file, int check_line) {
    if (!hdr || !user_ptr || hdr->extra_size == 0)
        return 0;

    size_t aligned_user_size = align_up(hdr->user_size);
    char* extra_start = (char*)user_ptr + aligned_user_size;

    // 检查整个额外内存区域是否被修改
    int first, last, count;
    find_corruption_range(extra_start, EXTRA_MEM_PATTERN, hdr->extra_size, &first, &last, &count);

    if (count > 0) {
        char error_buf[1024];
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

        SAFE_APPEND("Memory overflow (extra memory corruption) detected!%s", "\n");
        SAFE_APPEND("Memory header: %s", "\n");
        SAFE_APPEND("  allocated at: %s:%d\n", hdr->file, hdr->line);
        SAFE_APPEND("  checked at:   %s:%d\n", check_file, check_line);
        SAFE_APPEND("  user size:    %zu bytes\n", hdr->user_size);
        SAFE_APPEND("  extra size:   %zu bytes (%.0f%% of user size)\n", hdr->extra_size,
                    (hdr->user_size > 0) ? (hdr->extra_size * 100.0 / hdr->user_size) : 0);
        SAFE_APPEND("  user ptr:     %p\n", user_ptr);
        SAFE_APPEND("  extra memory: %p to %p\n", (void*)extra_start, (void*)(extra_start + hdr->extra_size - 1));
        SAFE_APPEND("  corrupted range: offset %d to %d (total %d bytes)\n", first, last, count);
        SAFE_APPEND("Likely cause: Buffer overflow beyond the end of allocated memory.%s", "\n");

        error_buf[buf_size - 1] = '\0';
        lmmp_abort(LAMMP_ERROR_OUT_OF_BOUNDS, error_buf, check_file, check_line);
        return 1;
    }
    return 0;
}

// === 全面检查内存块（头部魔数 + 额外内存）===
static inline int check_memory_block_integrity(MemHeader* hdr, void* user_ptr, const char* check_file, int check_line) {
    if (!hdr || !user_ptr)
        return 0;

    // 检查头部魔数是否被破坏（可能由下溢或野指针导致）
    if (hdr->magic != MEM_MAGIC) {
        char error_buf[256];
        snprintf(error_buf, sizeof(error_buf),
                 "Memory header corruption detected! Magic: 0x%08x (expected 0x%08x)\n"
                 "Possible underflow or invalid pointer.",
                 hdr->magic, MEM_MAGIC);
        lmmp_abort(LAMMP_ERROR_OUT_OF_BOUNDS, error_buf, check_file, check_line);
        return 1;
    }

    // 检查额外内存是否被溢出破坏
    return check_extra_memory_overflow(hdr, user_ptr, check_file, check_line);
}

// === 调试版 malloc ===
static inline void* lmmp_alloc_debug(size_t size, const char* file, int line) {
    if (size == 0)
        return NULL;

    // 计算额外分配的内存大小
    size_t extra_size = (size * MORE_ALLOC_TIMES) / 10;
    extra_size = align_up(extra_size);

    size_t header_size = align_up(sizeof(MemHeader));
    size_t aligned_user_size = align_up(size);
    // 总大小 = 头部 + 用户内存（对齐后） + 额外内存
    size_t total_size = header_size + aligned_user_size + extra_size;

    void* base = heap_alloc_func(total_size);
    if (!base) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Memory allocation failed (size: %zu bytes, extra: %zu bytes)", size, extra_size);
        lmmp_abort(LAMMP_ERROR_MEMORY_ALLOC_FAILURE, msg, file, line);
        return NULL;
    }

    memset(base, 0, total_size);

    MemHeader* hdr = (MemHeader*)base;
    void* user_ptr = (char*)base + header_size;
    void* extra_mem = (char*)user_ptr + aligned_user_size;

    hdr->user_size = size;
    hdr->total_size = total_size;
    hdr->extra_size = extra_size;
    hdr->file = file;
    hdr->line = line;
    hdr->magic = MEM_MAGIC;

    // 用户内存填充未初始化标记 (0xCD)
    memset(user_ptr, 0xCD, aligned_user_size);
    // 额外内存填充检测模式 (0xAA)
    memset(extra_mem, EXTRA_MEM_PATTERN, extra_size);

    return user_ptr;
}

// === 调试版 free ===
static inline void lmmp_free_debug(void* ptr, const char* file, int line) {
    if (!ptr)
        return;

    size_t header_size = align_up(sizeof(MemHeader));
    MemHeader* hdr = (MemHeader*)((char*)ptr - header_size);

    // 检查内存完整性（触发 lmmp_abort 若溢出或头部损坏）
    check_memory_block_integrity(hdr, ptr, file, line);

    // 标记已释放 (0xDD)
    memset(hdr, 0xDD, hdr->total_size);
    heap_free_func(hdr);
}

// === 调试版 realloc ===
static inline void* lmmp_realloc_debug(void* ptr, size_t new_size, const char* file, int line) {
    if (!ptr)
        return lmmp_alloc_debug(new_size, file, line);
    if (new_size == 0) {
        lmmp_free_debug(ptr, file, line);
        return NULL;
    }

    size_t header_size = align_up(sizeof(MemHeader));
    MemHeader* old_hdr = (MemHeader*)((char*)ptr - header_size);

    // 检查原内存块完整性
    check_memory_block_integrity(old_hdr, ptr, file, line);

    void* new_ptr = lmmp_alloc_debug(new_size, file, line);
    if (!new_ptr)
        return NULL;

    size_t copy_size = (old_hdr->user_size < new_size) ? old_hdr->user_size : new_size;
    memcpy(new_ptr, ptr, copy_size);

    lmmp_free_debug(ptr, file, line);
    return new_ptr;
}

// === 显式检查宏 ===
static inline int check_memory_overflow(void* ptr, const char* file, int line) {
    if (!ptr)
        return 0;
    size_t header_size = align_up(sizeof(MemHeader));
    MemHeader* hdr = (MemHeader*)((char*)ptr - header_size);
    return check_memory_block_integrity(hdr, ptr, file, line);
}

#define CHECK_OVERFLOW(ptr) check_memory_overflow(ptr, __FILE__, __LINE__)

#undef MORE_ALLOC_TIMES
#undef SAFE_APPEND
#undef MEM_MAGIC
#undef ALIGNMENT
#undef EXTRA_MEM_PATTERN

#endif  // __LAMMP_SAFE_MEMORY_H__