/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../../include/lammp/lmmp.h"
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>

static atomic_uintptr_t lmmp_abort_func;

lmmp_abort_fn lmmp_set_abort_fn(lmmp_abort_fn func) {
    uintptr_t old = atomic_exchange(&lmmp_abort_func, (uintptr_t)func);
    return (lmmp_abort_fn)old;
}

static const char* type_to_str(lmmp_error_t type) {
    switch (type) {
        case LAMMP_ERROR_ASSERT_FAILURE:
            return "ASSERT_FAILURE";
        case LAMMP_ERROR_DEBUG_ASSERT_FAILURE:
            return "DEBUG_ASSERT_FAILURE";
        case LAMMP_ERROR_PARAM_ASSERT_FAILURE:
            return "PARAM_ASSERT_FAILURE";
        case LAMMP_ERROR_MEMORY_ALLOC_FAILURE:
            return "MEMORY_ALLOC_FAILURE";
        case LAMMP_ERROR_MEMORY_FREE_FAILURE:
            return "MEMORY_FREE_FAILURE";
        case LAMMP_ERROR_OUT_OF_BOUNDS:
            return "OUT_OF_BOUNDS";
        case LAMMP_ERROR_MEMORY_LEAK:
            return "MEMORY_LEAK";
        case LAMMP_ERROR_UNEXPECTED_ERROR:
            return "UNEXPECTED_ERROR";
        default:
            return "UNKNOWN_TYPE";
    }
}

void lmmp_abort(lmmp_error_t type, const char* msg, const char* func, int line) {
    uintptr_t curr = atomic_load(&lmmp_abort_func);
    lmmp_abort_fn fn = (lmmp_abort_fn)curr;
    if (fn != NULL) {
        fn(type, msg, func, line);
    } else {
        fprintf(stderr, "LAMMP abort at [%s]:%d\n", func, line);
        fprintf(stderr, "Abort type: %s, abort msg: \n%s\n", type_to_str(type), msg);
        fflush(stderr);
        abort();
    }
}