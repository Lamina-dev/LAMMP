#ifndef __LAMMP_SAFE_MEMORY_H__
#define __LAMMP_SAFE_MEMORY_H__

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// === 内存块头部结构 ===
typedef struct {
    size_t user_size;   // 用户请求的大小
    size_t total_size;  // 总分配大小（含头、哨兵）
    uint32_t magic;     // 魔数，用于验证有效性
    const char* file;   // 分配发生的文件
    int line;           // 分配发生的行号
} MemHeader;

#define MEM_MAGIC 0xDEADBEEF

// === 哨兵配置 ===
#define GUARD_SIZE 16

static const char FRONT_GUARD_PATTERN[GUARD_SIZE] = "FONT_GUARD_1234";
static const char REAR_GUARD_PATTERN[GUARD_SIZE] = "REAR_GUARD_4321";

// === 对齐工具 ===
#define ALIGNMENT 8
static inline size_t align_up(size_t size) { return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1); }

// === 检查前哨兵（下溢）===
static inline int check_front_guard_underflow(MemHeader* hdr, void* user_ptr, const char* check_file, int check_line) {
    if (!hdr || !user_ptr)
        return 0;

    size_t header_size = align_up(sizeof(MemHeader));
    char* front_guard = (char*)hdr + header_size;

    if (memcmp(front_guard, FRONT_GUARD_PATTERN, GUARD_SIZE) != 0) {
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

        SAFE_APPEND("%s%s", "Memory underflow (front guard corruption) detected!", "\n");
        SAFE_APPEND("Allocated at: %s:%d\n", hdr->file, hdr->line);
        SAFE_APPEND("Checked at:   %s:%d\n", check_file, check_line);
        SAFE_APPEND("User size:    %zu bytes\n", hdr->user_size);
        SAFE_APPEND("User ptr:     %p\n", user_ptr);
        SAFE_APPEND("Front guard:  %p\n", (void*)front_guard);

        char* front_data = front_guard;
        int corruption_count = 0;
        for (int i = 0; i < GUARD_SIZE; i++) {
            if (front_data[i] != FRONT_GUARD_PATTERN[i])
                corruption_count++;
        }
        SAFE_APPEND("Corrupted bytes: %d\n", corruption_count);

        int printed = 0;
        for (int i = 0; i < GUARD_SIZE && printed < 8; i++) {
            if (front_data[i] != FRONT_GUARD_PATTERN[i]) {
                char exp =
                    (FRONT_GUARD_PATTERN[i] >= 32 && FRONT_GUARD_PATTERN[i] < 127) ? FRONT_GUARD_PATTERN[i] : '.';
                char got = (front_data[i] >= 32 && front_data[i] < 127) ? front_data[i] : '.';
                SAFE_APPEND("  [Offset %2d] Expected 0x%02X('%c'), Got 0x%02X('%c')\n", i,
                            (unsigned char)FRONT_GUARD_PATTERN[i], exp, (unsigned char)front_data[i], got);
                printed++;
            }
        }
        SAFE_APPEND("%s%s", "Likely cause: Writing before start of buffer (e.g., ptr[-1] = x).", "\n");
        error_buf[buf_size - 1] = '\0';

        lmmp_abort(LAMMP_OUT_OF_BOUNDS, error_buf, check_file, check_line);
        return 1;
    }
    return 0;
}

// === 检查后哨兵（上溢）===
static inline int check_rear_guard_overflow(MemHeader* hdr, void* user_ptr, const char* check_file, int check_line) {
    if (!hdr || !user_ptr)
        return 0;

    size_t aligned_user_size = align_up(hdr->user_size);
    char* rear_guard = (char*)user_ptr + aligned_user_size;

    if (memcmp(rear_guard, REAR_GUARD_PATTERN, GUARD_SIZE) != 0) {
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

        SAFE_APPEND("%s%s", "Memory overflow (rear guard corruption) detected!", "\n");
        SAFE_APPEND("Allocated at: %s:%d\n", hdr->file, hdr->line);
        SAFE_APPEND("Checked at:   %s:%d\n", check_file, check_line);
        SAFE_APPEND("User size:    %zu bytes\n", hdr->user_size);
        SAFE_APPEND("User ptr:     %p\n", user_ptr);
        SAFE_APPEND("Rear guard:   %p\n", (void*)rear_guard);

        char* rear_data = rear_guard;
        int first_corruption = -1;
        int corruption_count = 0;
        for (int i = 0; i < GUARD_SIZE; i++) {
            if (rear_data[i] != REAR_GUARD_PATTERN[i]) {
                if (first_corruption == -1)
                    first_corruption = i;
                corruption_count++;
            }
        }
        SAFE_APPEND("Corrupted bytes: %d\n", corruption_count);

        int printed = 0;
        for (int i = 0; i < GUARD_SIZE && printed < 8; i++) {
            if (rear_data[i] != REAR_GUARD_PATTERN[i]) {
                char exp = (REAR_GUARD_PATTERN[i] >= 32 && REAR_GUARD_PATTERN[i] < 127) ? REAR_GUARD_PATTERN[i] : '.';
                char got = (rear_data[i] >= 32 && rear_data[i] < 127) ? rear_data[i] : '.';
                SAFE_APPEND("  [Offset %2d] Expected 0x%02X('%c'), Got 0x%02X('%c')\n", i,
                            (unsigned char)REAR_GUARD_PATTERN[i], exp, (unsigned char)rear_data[i], got);
                printed++;
            }
        }

        if (corruption_count == 1 && first_corruption == 0) {
            SAFE_APPEND("%s%s", "Likely cause: Missing null terminator in string (e.g., strcpy without space for '\\0').", "\n");
        } else if (corruption_count > 1 && first_corruption == 0) {
            SAFE_APPEND("%s%s", "Likely cause: Buffer/array overflow (e.g., writing beyond allocated size).", "\n");
        } else {
            SAFE_APPEND("%s%s", "Likely cause: Arbitrary memory corruption near end of buffer.", "\n");
        }

        error_buf[buf_size - 1] = '\0';
        lmmp_abort(LAMMP_OUT_OF_BOUNDS, error_buf, check_file, check_line);
        return 1;
    }
    return 0;
}

// === 全面检查内存块（前后哨兵）===
static inline int check_memory_block_integrity(MemHeader* hdr, void* user_ptr, const char* check_file, int check_line) {
    if (!hdr || !user_ptr)
        return 0;
    if (hdr->magic != MEM_MAGIC) {
        // 魔数错误通常表示 double-free 或野指针，可单独处理，此处暂不触发 LAMMP_OUT_OF_BOUNDS
        return 0;
    }
    int underflow = check_front_guard_underflow(hdr, user_ptr, check_file, check_line);
    int overflow = check_rear_guard_overflow(hdr, user_ptr, check_file, check_line);
    return underflow || overflow;
}

// === 调试版 malloc ===
static inline void* lmmp_malloc_debug(size_t size, const char* file, int line) {
    if (size == 0)
        return NULL;

    size_t header_size = align_up(sizeof(MemHeader));
    size_t aligned_user_size = align_up(size);
    size_t total_size = header_size + GUARD_SIZE + aligned_user_size + GUARD_SIZE;

    void* base = malloc(total_size);
    if (!base) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Memory allocation failed (size: %zu bytes)", size);
        lmmp_abort(LAMMP_MEMORY_ALLOC_FAILURE, msg, file, line);
        return NULL;
    }

    memset(base, 0, total_size);

    MemHeader* hdr = (MemHeader*)base;
    void* front_guard = (char*)base + header_size;
    void* user_ptr = (char*)front_guard + GUARD_SIZE;
    void* rear_guard = (char*)user_ptr + aligned_user_size;

    hdr->user_size = size;
    hdr->total_size = total_size;
    hdr->file = file;
    hdr->line = line;
    hdr->magic = MEM_MAGIC;

    memcpy(front_guard, FRONT_GUARD_PATTERN, GUARD_SIZE);
    memcpy(rear_guard, REAR_GUARD_PATTERN, GUARD_SIZE);
    memset(user_ptr, 0xCD, aligned_user_size);  // 未初始化内存标记

    return user_ptr;
}

// === 调试版 free ===
static inline void lmmp_free_debug(void* ptr, const char* file, int line) {
    if (!ptr)
        return;

    size_t header_size = align_up(sizeof(MemHeader));
    MemHeader* hdr = (MemHeader*)((char*)ptr - header_size - GUARD_SIZE);

    if (hdr->magic != MEM_MAGIC) {
        // 可选：这里也可调用 lmmp_abort，但类型不是 OUT_OF_BOUNDS
        return;
    }

    // 检查内存完整性（触发 lmmp_abort 若越界）
    check_memory_block_integrity(hdr, ptr, file, line);

    // 标记已释放
    memset(hdr, 0xDD, hdr->total_size);
    free(hdr);
}

// === 调试版 realloc ===
static inline void* lmmp_realloc_debug(void* ptr, size_t new_size, const char* file, int line) {
    if (!ptr)
        return lmmp_malloc_debug(new_size, file, line);
    if (new_size == 0) {
        lmmp_free_debug(ptr, file, line);
        return NULL;
    }

    size_t header_size = align_up(sizeof(MemHeader));
    MemHeader* old_hdr = (MemHeader*)((char*)ptr - header_size - GUARD_SIZE);

    if (old_hdr->magic != MEM_MAGIC) {
        return NULL;
    }

    // 检查旧块
    check_memory_block_integrity(old_hdr, ptr, file, line);

    void* new_ptr = lmmp_malloc_debug(new_size, file, line);
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
    MemHeader* hdr = (MemHeader*)((char*)ptr - header_size - GUARD_SIZE);
    if (hdr->magic != MEM_MAGIC)
        return 0;
    return check_memory_block_integrity(hdr, ptr, file, line);
}

#define CHECK_OVERFLOW(ptr) check_memory_overflow(ptr, __FILE__, __LINE__)

#undef SAFE_APPEND
#undef MEM_MAGIC
#undef GUARD_SIZE
#undef FRONT_GUARD_PATTERN
#undef REAR_GUARD_PATTERN
#undef ALIGNMENT

#endif  // __LAMMP_SAFE_MEMORY_H__