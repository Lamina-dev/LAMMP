#include "../../include/lammp/lmmpn.h"

/* 
pmarker is a head pointer to a linked list of allocated memory blocks.
Each allocated block has a header of size HSIZE, which is used to store the
next pointer of the block. The actual data starts at (mp_byte_t*)p + HSIZE.
*/

#define HSIZE sizeof(void*)

void* lmmp_temp_alloc_(void** pmarker, size_t size) {
    void* p = lmmp_alloc(size + HSIZE);
    *(void**)p = *pmarker;
    *pmarker = p;
    return (mp_byte_t*)p + HSIZE;
}

void lmmp_temp_free_(void* marker) {
/*
   Free all allocated memory blocks in the linked list pointed to by pmarker.
*/
    while (marker) {
        void* next = *(void**)marker;
        lmmp_free(marker);
        marker = next;
    }
}

void* lmmp_alloc(size_t size) {
    void* ret = malloc(size);
    lmmp_assert(ret);
    return ret;
}

void* lmmp_realloc(void* oldptr, size_t new_size) {
    void* ret = realloc(oldptr, new_size);
    lmmp_assert(ret);
    return ret;
}

void lmmp_free(void* ptr) { free(ptr); }
