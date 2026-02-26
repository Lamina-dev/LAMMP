#include "../../include/lammp/lmmpn.h"
#undef lmmp_alloc
#undef lmmp_realloc
#undef lmmp_free
#include "../../include/lammp/impl/safe_memory.h"

#define HSIZE sizeof(void*)

static lmmp_heap_alloc_fn heap_alloc_func = malloc;
static lmmp_heap_free_fn heap_free_func = free;
static lmmp_realloc_fn realloc_func = realloc;

static void* default_stack_begin = NULL;
static void* default_stack_end = NULL;
static void* default_stack_top = NULL;

void lmmp_default_stack_reset(size_t size) {
    if (size) {
        if (default_stack_begin)
            heap_free_func(default_stack_begin);
        default_stack_begin = heap_alloc_func(size);
        default_stack_end = (mp_byte_t*)default_stack_begin + size;
        default_stack_top = default_stack_begin;
    } else {
        if (default_stack_begin)
            heap_free_func(default_stack_begin);
        default_stack_begin = NULL;
        default_stack_end = NULL;
        default_stack_top = NULL;
    }
}

static inline void* lmmp_default_stack_get_top(void) {
    if (default_stack_begin == NULL)
        lmmp_default_stack_reset(LAMMP_DEFAULT_STACK_SIZE);
    return default_stack_top;
}

static inline void lmmp_default_stack_set_top(void* top) {
#if DEFAULT_STACK_OVERFLOW_CHECK == 1
    if (top > default_stack_end) {
        char msg[128];
        snprintf(msg, sizeof(msg), "default stack overflow (trying to allocate: %zu bytes, stack remaining: %zu bytes)",
                 (size_t)((mp_byte_t*)top - (mp_byte_t*)default_stack_top),
                 (size_t)((mp_byte_t*)default_stack_end - (mp_byte_t*)default_stack_top));
        lmmp_abort(LAMMP_MEMORY_ALLOC_FAILURE, msg, __FILE__, __LINE__);
    }
#endif
    default_stack_top = top;
}

static lmmp_stack_get_top_fn stack_get_top_func = lmmp_default_stack_get_top;
static lmmp_stack_set_top_fn stack_set_top_func = lmmp_default_stack_set_top;

lmmp_heap_alloc_fn lmmp_set_heap_alloc_fn(lmmp_heap_alloc_fn func) {
    lmmp_heap_alloc_fn old_func = heap_alloc_func;
    heap_alloc_func = func;
    return old_func;
}

lmmp_heap_free_fn lmmp_set_heap_free_fn(lmmp_heap_free_fn func) {
    lmmp_heap_free_fn old_func = heap_free_func;
    heap_free_func = func;
    return old_func;
}

lmmp_realloc_fn lmmp_set_realloc_fn(lmmp_realloc_fn func) {
    lmmp_realloc_fn old_func = realloc_func;
    realloc_func = func;
    return old_func;
}

lmmp_stack_get_top_fn lmmp_set_stack_get_top_fn(lmmp_stack_get_top_fn func) {
    lmmp_stack_get_top_fn old_func = stack_get_top_func;
    stack_get_top_func = func;
    return old_func;
}

lmmp_stack_set_top_fn lmmp_set_stack_set_top_fn(lmmp_stack_set_top_fn func) {
    lmmp_stack_set_top_fn old_func = stack_set_top_func;
    stack_set_top_func = func;
    return old_func;
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
/*
 *  Free all allocated memory blocks in the linked list pointed to by pmarker.
 */
    stack_set_top_func(marker);
}

static inline void lmmp_chech_memory(size_t size, const char* file, int line) {
    char msg[64];
    snprintf(msg, sizeof(msg), "Memory allocation failed (size: %zu bytes)", size);
    lmmp_abort(LAMMP_ERROR_MEMORY_ALLOC_FAILURE, msg, file, line);
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
