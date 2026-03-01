#include <stdio.h>
#include "../../include/lammp/lmmp.h"

mp_byte_t stack_memory[10000];
mp_byte_t* top = stack_memory;

void* get_top(void) {
    return top;
}

void set_top(void* new_top) {
    top = (mp_byte_t*)new_top;
}

lmmp_stack_alloctor_t stack_allocator;

int main() {
    mp_size_t size = 100;
    mp_ptr a = (mp_ptr)lmmp_stack_alloc(size * sizeof(mp_limb_t));
    mp_ptr b = (mp_ptr)lmmp_stack_alloc(1001 * sizeof(mp_limb_t));
    memset(a, 0, 10 * sizeof(mp_limb_t));
    lmmp_stack_free(b);
    lmmp_global_deinit();
    lmmp_stack_free(a);
    return 0;
}
