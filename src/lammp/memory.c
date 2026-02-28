#include "../../include/lammp/lmmpn.h"
#include "../../include/lammp/impl/default_stack.h"

#undef lmmp_alloc
#undef lmmp_realloc
#undef lmmp_free
#undef lmmp_stack_alloc
#undef lmmp_stack_free
#undef lmmp_leak_tracker
#define HSIZE sizeof(void*)

static lmmp_heap_alloctor_t global_heap = {
    malloc,
    free,
    realloc,
};

static int heap_alloc_count = 0;

#define heap_alloc_func global_heap.alloc
#define heap_free_func global_heap.free
#define realloc_func global_heap.realloc

static void* default_stack_begin = NULL;
static void* default_stack_end = NULL;
static void* default_stack_top = NULL;

static int is_default_stack = 1; // 0: not using default stack, 1: using default stack

/* global stack only uses memory space without managing it */
static lmmp_stack_alloctor_t global_stack = {
    NULL, NULL, lmmp_default_stack_get_top, lmmp_default_stack_set_top,
};

#define stack_get_top_func() (global_stack.get())
#define stack_set_top_func(top) (global_stack.set(top))

void lmmp_default_stack_reset(size_t size) {
    if (size) {
        is_default_stack = 1;
        if (!((mp_byte_t*)default_stack_begin + size < (mp_byte_t*)default_stack_end)) {
            default_stack_begin = realloc_func(default_stack_begin, size);
            default_stack_end = (mp_byte_t*)default_stack_begin + size;
        }
        if (default_stack_begin == NULL) {
            lmmp_abort(LAMMP_ERROR_MEMORY_ALLOC_FAILURE, "Default stack allocation failed", __FILE__, __LINE__);
        }
        default_stack_top = default_stack_begin;
        global_stack.begin = default_stack_begin;
        global_stack.end = default_stack_end;
        global_stack.get = lmmp_default_stack_get_top;
        global_stack.set = lmmp_default_stack_set_top;
    } else {
#if LAMMP_DEBUG_MEMORY_LEAK == 1
        if (default_stack_top != default_stack_begin) {
            char msg[128];
            snprintf(msg, sizeof(msg), "Default stack allocator is not empty. top: %p, begin: %p, end: %p\n",
                     stack_get_top_func(), global_stack.begin, global_stack.end);
            lmmp_abort(LAMMP_ERROR_MEMORY_LEAK, msg, __FILE__, __LINE__);
        }
#endif
        if (default_stack_begin)
            heap_free_func(default_stack_begin);
        default_stack_begin = NULL;
        default_stack_end = NULL;
        default_stack_top = NULL;
    }
}

static void* lmmp_default_stack_get_top(void) {
    if (default_stack_begin == NULL)
        lmmp_default_stack_reset(LAMMP_DEFAULT_STACK_SIZE);
    return default_stack_top;
}

static void lmmp_default_stack_set_top(void* top) {
#if LAMMP_DEBUG_STACK_OVERFLOW_CHECK == 1
    if (top < default_stack_begin || top > default_stack_end) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Default stack overflow, trying to set top: %p , begin: %p , end: %p", top,
                 default_stack_begin, default_stack_end);
        lmmp_abort(LAMMP_ERROR_MEMORY_ALLOC_FAILURE, msg, __FILE__, __LINE__);
    }
#endif
    default_stack_top = top;

}

void lmmp_set_heap_alloctor(const lmmp_heap_alloctor_t* heap) {
    if (heap == NULL) return;
    lmmp_default_stack_reset(0);
#if LAMMP_DEBUG_MEMORY_LEAK == 1
    if (heap_alloc_count != 0) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Older heap allocations not freed: %d block(s)", heap_alloc_count);
        lmmp_abort(LAMMP_ERROR_MEMORY_LEAK, msg, __FILE__, __LINE__);
    }
#endif
    global_heap = *heap;
}

void lmmp_set_stack_alloctor(const lmmp_stack_alloctor_t* stack) {
    if (stack == NULL) {
        if (default_stack_begin == NULL)
            heap_alloc_func(LAMMP_DEFAULT_STACK_SIZE);
        global_stack.begin = default_stack_begin;
        global_stack.end = default_stack_end;
        global_stack.get = lmmp_default_stack_get_top;
        global_stack.set = lmmp_default_stack_set_top;
        return;
    }
    if (stack->begin > stack->end) {
        lmmp_abort(LAMMP_ERROR_PARAM_ASSERT_FAILURE, "Stack allocator begin address is greater than end address",
                   __FILE__, __LINE__);
    }
#if LAMMP_DEBUG_MEMORY_LEAK == 1
    if (stack_get_top_func() != global_stack.begin) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Older stack allocator is not empty. top: %p, begin: %p, end: %p\n", 
                 stack_get_top_func(), global_stack.begin, global_stack.end);
        lmmp_abort(LAMMP_ERROR_MEMORY_LEAK, msg, __FILE__, __LINE__);
    }
#endif
    global_stack = *stack;
    is_default_stack = 0;
}

void* lmmp_temp_heap_alloc_(void** pmarker, size_t size) {
/*
 * pmarker is a head pointer to a linked list of allocated memory blocks.
 * Each allocated block has a header of size HSIZE, which is used to store the
 * next pointer of the block. The actual data starts at (mp_byte_t*)p + HSIZE.
 */
    void* p = heap_alloc_func(size + HSIZE);
    *(void**)p = *pmarker;
    *pmarker = p;
    return (mp_byte_t*)p + HSIZE;
}

void* lmmp_temp_stack_alloc_(void** pmarker, size_t size) {
/*
 * On the first call, *pmarker is a null pointer. 
 * We will use *pmarker to record the stack frame at this time. 
 * When allocating memory subsequently, we will not modify *pmarker. 
 * Until all stack memory is finally released, we will move to the initial stack position at once, 
 * which is the position recorded by *pmarker.
 */
    void* p = stack_get_top_func();
    stack_set_top_func((mp_byte_t*)p + size);
    if (*pmarker == NULL)
        *pmarker = p;
    return p;
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

void lmmp_temp_stack_free_(void* marker) {
    stack_set_top_func(marker);
}

#include "../../include/lammp/impl/safe_memory.h"

static inline void lmmp_chech_memory(size_t size, const char* file, int line) {
    char msg[64];
    snprintf(msg, sizeof(msg), "Memory allocation failed (size: %zu bytes)", size);
    lmmp_abort(LAMMP_ERROR_MEMORY_ALLOC_FAILURE, msg, file, line);
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

void lmmp_leak_tracker(const char* file, int line) {
    char msg[192] = {0};
    int offset = 0;
    const int max_len = sizeof(msg) - 1;
    int t = 0;
    if (stack_get_top_func() != global_stack.begin) {
        offset += snprintf(msg + offset, max_len - offset, "Stack allocator is not empty. top: %p begin: %p end: %p\n",
                           stack_get_top_func(), global_stack.begin, global_stack.end);
        t = 1;
    }
    if (heap_alloc_count != 0) {
        offset += snprintf(msg + offset, max_len - offset, "Heap allocations not freed: %d block(s)\n", heap_alloc_count);
        t = 1;
    }
    if (t) {
        lmmp_abort(LAMMP_ERROR_MEMORY_LEAK, msg, file, line);
    }
}

#if LAMMP_DEBUG_MEMORY_CHECK == 1
void* lmmp_alloc(size_t size, const char* file, int line) { 
    if (size) {
        void* ret = lmmp_alloc_debug(size, file, line);
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
            lmmp_chech_memory(size, __FILE__, __LINE__);
#if LAMMP_DEBUG_MEMORY_LEAK == 1
        heap_alloc_count++;
#endif
        return ret;
    }
    return NULL;
}
#endif

#if LAMMP_DEBUG_MEMORY_CHECK == 1
void* lmmp_realloc(void* oldptr, size_t new_size, const char* file, int line) {
    void* ret = lmmp_realloc_debug(oldptr, new_size, file, line);
    return ret;
}
#else
void* lmmp_realloc(void* oldptr, size_t new_size) {
    void* ret = realloc_func(oldptr, new_size);
    if (ret == NULL) 
        lmmp_chech_memory(new_size, __FILE__, __LINE__);
    return ret;
}
#endif

#if LAMMP_DEBUG_MEMORY_CHECK == 1
void lmmp_free(void* ptr, const char* file, int line) {
    if (ptr) {
        lmmp_free_debug(ptr, file, line);
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
#define SIZE_SIZE sizeof(size_t)

void* lmmp_stack_alloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    size_t total_size = SIZE_SIZE + size;
    void* old_top = stack_get_top_func();
    void* new_top = (mp_byte_t*)old_top + total_size;
#if LAMMP_DEBUG_STACK_OVERFLOW_CHECK == 1
    if (new_top > global_stack.end) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Stack overflow (trying to allocate: %zu bytes, stack remaining: %zu bytes)",
                 total_size, (size_t)((mp_byte_t*)global_stack.end - (mp_byte_t*)old_top));
        lmmp_abort(LAMMP_ERROR_MEMORY_ALLOC_FAILURE, msg, __FILE__, __LINE__);
    }
#endif // LAMMP_DEBUG_STACK_OVERFLOW_CHECK == 1
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
        snprintf(msg, sizeof(msg),
                 "Invalid stack pointer (trying to free: %p ; stack start: %p , stack end: %p )",
                 ptr, global_stack.begin, global_stack.end);
        lmmp_abort(LAMMP_ERROR_MEMORY_FREE_FAILURE, msg, __FILE__, __LINE__);
    }
#endif // LAMMP_DEBUG_STACK_OVERFLOW_CHECK == 1
    void* old_top = stack_get_top_func();
    size_t total_size = *(size_t*)((mp_byte_t*)ptr - SIZE_SIZE);
    void* new_top = (mp_byte_t*)old_top - total_size;
#if LAMMP_DEBUG_STACK_OVERFLOW_CHECK == 1
    if (new_top < global_stack.begin || new_top > global_stack.end) {
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "Stack underflow (trying to free: %p , size: %zu bytes ; stack start: %p , stack end: %p ) \n%s",
                 ptr, total_size - SIZE_SIZE, global_stack.begin, global_stack.end,
                 "Likely cause: Previous stack buffer overflow corrupted the memory header.");
        lmmp_abort(LAMMP_ERROR_MEMORY_FREE_FAILURE, msg, __FILE__, __LINE__);
    }
#endif // LAMMP_DEBUG_STACK_OVERFLOW_CHECK == 1
    stack_set_top_func(new_top);
}

#else

typedef struct {
    size_t total_size;         // 总分配大小（原有）
    size_t extra_size;         // 额外分配的魔数区域大小
    size_t magic_addr_offset;  // 魔数起始地址相对于old_top的偏移
    void* last_ptr;            // 上一次分配的指针
    const char* file;          // 分配时的文件名
    int line;                  // 分配时的文件行号
} StackHeader;

#define HEADER_SIZE sizeof(StackHeader) 

#define MAGIC_NUMBER 0xDEADBEEF
#define MAGIC_SIZE sizeof(unsigned int) 

static void* global_stack_last_ptr = NULL;  // 最后一次分配的指针

void* lmmp_stack_alloc(size_t size, const char* file, int line) {
    if (size == 0) {
        return NULL;
    }

    size_t base_data_size = size;
    size_t base_total_size = HEADER_SIZE + base_data_size;

    size_t extra_size = (base_total_size * LAMMP_MEMORY_MORE_ALLOC_TIMES) / 10;
    if (extra_size < MAGIC_SIZE) {
        extra_size = MAGIC_SIZE;
    }
    size_t total_size = base_total_size + extra_size;

    void* old_top = stack_get_top_func();
    void* new_top = (mp_byte_t*)old_top + total_size;
    if (new_top > global_stack.end) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Stack overflow (alloc: %zu bytes, remaining: %zu bytes)", total_size,
                 (size_t)((mp_byte_t*)global_stack.end - (mp_byte_t*)old_top));
        lmmp_abort(LAMMP_ERROR_MEMORY_ALLOC_FAILURE, msg, file, line);
    }

    StackHeader* header = (StackHeader*)old_top;
    header->total_size = total_size;
    header->extra_size = extra_size;
    header->magic_addr_offset = base_total_size;  // 魔数起始地址 = old_top + base_total_size
    header->last_ptr = global_stack_last_ptr;
    header->file = file;
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

void lmmp_stack_free(void* ptr, const char* file, int line) {
    if (ptr == NULL) {
        return;
    }
    if (ptr < global_stack.begin || ptr >= global_stack.end) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Invalid stack pointer, trying to free %p (stack start: %p, end: %p)", ptr,
                 global_stack.begin, global_stack.end);
        lmmp_abort(LAMMP_ERROR_MEMORY_FREE_FAILURE, msg, file, line);
    }
    if (ptr != global_stack_last_ptr) {
        char msg[96];
        snprintf(msg, sizeof(msg), "Invalid stack pointer. Expected %p, but try to free %p", global_stack_last_ptr,
                 ptr);
        lmmp_abort(LAMMP_ERROR_MEMORY_FREE_FAILURE, msg, file, line);
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
        char error_buf[640];
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

        SAFE_APPEND("Stack buffer overflow detected! Magic number corrupted at %p (ptr: %p, size: %zu)\n",
                    magic_addr, ptr, (mp_byte_t*)magic_addr - (mp_byte_t*)ptr);
        SAFE_APPEND("Stack buffer header:%s", "\n");
        SAFE_APPEND("  allocated at: %s:%d\n", header->file, header->line);
        SAFE_APPEND("  total size:   %zu bytes\n", header->total_size);
        SAFE_APPEND("  extra size:   %zu bytes (%.0f%% of user size)\n", 
                    header->extra_size, LAMMP_MEMORY_MORE_ALLOC_TIMES * 10.0);
        SAFE_APPEND("  magic offset: %zu\n", header->magic_addr_offset);
        SAFE_APPEND("  last ptr:     %p\n", header->last_ptr);
        lmmp_abort(LAMMP_ERROR_OUT_OF_BOUNDS, error_buf, file, line);
    }

    void* old_top = stack_get_top_func();
    void* new_top = (mp_byte_t*)old_top - total_size;
    if (new_top < global_stack.begin || new_top > global_stack.end) {
        char msg[192];
        snprintf(msg, sizeof(msg), "Stack underflow (free: %p, size: %zu; stack start: %p, end: %p)", ptr,
                 (mp_byte_t*)magic_addr - (mp_byte_t*)ptr, global_stack.begin, global_stack.end);
        lmmp_abort(LAMMP_ERROR_MEMORY_FREE_FAILURE, msg, file, line);
    }

    if (new_top != global_stack.begin) {
        if ((mp_byte_t*)new_top + HEADER_SIZE > (mp_byte_t*)global_stack.end) {
            lmmp_abort(LAMMP_ERROR_MEMORY_FREE_FAILURE, "Invalid previous block header", file, line);
        }
    }
    stack_set_top_func(new_top);
}
#endif // LAMMP_DEBUG_MEMORY_CHECK != 1

void lmmp_global_deinit(void) {
    lmmp_default_stack_reset(0);
}