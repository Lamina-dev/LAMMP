/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

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
    mp_ptr a = (mp_ptr)lmmp_alloc(1);
    mp_ptr b = (mp_ptr)lmmp_alloc(2);
    printf("%p %p\n", (void*)a, (void*)b);
    lmmp_free(b);
    lmmp_free(a);
    lmmp_global_deinit();
    return 0;
}
