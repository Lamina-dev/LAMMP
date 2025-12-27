#include "../../../include/lammp/alloc.h"
#include <assert.h>

void* lammp_alloc(size_t size, size_t alignment) {
    assert(alignment > 0 && ((alignment & (alignment - 1)) == 0)); 
    if (size == 0) {
        return NULL;
    }

#if defined(_WIN32)
    return _aligned_malloc(size, alignment);

#elif defined(__linux__)
    assert((alignment % DEFAULT_ALIGNMENT) == 0);
    void* aligned_ptr = NULL;
    int ret = posix_memalign(&aligned_ptr, alignment, size);
    return (ret == 0) ? aligned_ptr : NULL;

#else
    const size_t extra_size = alignment + DEFAULT_ALIGNMENT;
    const size_t total_size = size + extra_size;

    void* raw_ptr = malloc(total_size);
    if (raw_ptr == NULL) {
        return NULL;
    }

    uintptr_t raw_addr = (uintptr_t)raw_ptr;
    uintptr_t aligned_addr = raw_addr + DEFAULT_ALIGNMENT;
    aligned_addr = (aligned_addr + alignment - 1) & ~((uintptr_t)(alignment - 1));
    void* aligned_ptr = (void*)aligned_addr;

    void** raw_ptr_storage = (void**)(aligned_addr - DEFAULT_ALIGNMENT);
    *raw_ptr_storage = raw_ptr;

    return aligned_ptr;
#endif
}

void lammp_free(void* ptr) {
    if (ptr == NULL) {
        return;
    }
#if defined(_WIN32)
    _aligned_free(ptr);
#elif defined(__linux__)
    free(ptr);
#else
    uintptr_t aligned_addr = (uintptr_t)ptr;
    void** raw_ptr_storage = (void**)(aligned_addr - DEFAULT_ALIGNMENT);
    void* raw_ptr = *raw_ptr_storage;
    free(raw_ptr);
#endif
}

void* lammp_calloc(size_t elem_align, size_t elem_len) {
    assert(elem_align > 0 && ((elem_align & (elem_align - 1)) == 0));
    size_t size = elem_len * elem_align;
    void* buf = lammp_alloc(size, elem_align);
    if (buf == NULL) {
        return NULL;
    }
    memset(buf, 0, size);
    return buf;
}

void lammp_realloc(void** ptr, size_t buf_len, size_t elem_align, size_t elem_len) {
    assert(ptr != NULL);
    assert(elem_align > 0 && ((elem_align & (elem_align - 1)) == 0));
    assert(elem_len > buf_len);

    void* old_ptr = *ptr;                    
    size_t old_size = buf_len * elem_align;  
    size_t new_size = elem_len * elem_align;  
    void* new_ptr = NULL;

    new_ptr = lammp_alloc(new_size, elem_align); 
    if (new_ptr == NULL) {
        return;
    }

    if (old_ptr != NULL && old_size > 0) {
        memcpy(new_ptr, old_ptr, old_size);
    }

    size_t tail_clear_size = new_size - old_size;
    if (tail_clear_size > 0) {
        memset((char*)new_ptr + old_size, 0, tail_clear_size);
    }

    if (old_ptr != NULL) {
        lammp_free(old_ptr);  
    }

    *ptr = new_ptr;
}

void* lammp_talloc(size_t elem_align, size_t elem_len, size_t tail_zero_len) {
    assert(elem_align > 0 && ((elem_align & (elem_align - 1)) == 0));
    assert(tail_zero_len <= elem_len);
    if (tail_zero_len == 0) {
        return lammp_alloc(elem_len * elem_align, elem_align);
    }
    size_t total_size = elem_len * elem_align;
    void* ptr = lammp_alloc(total_size, elem_align);
    if (ptr == NULL) {
        return NULL;
    }

    char* tail_start_ptr = (char*)ptr + (elem_len - tail_zero_len) * elem_align;
    size_t tail_zero_size = tail_zero_len * elem_align;
    memset(tail_start_ptr, 0, tail_zero_size);
    return ptr;
}