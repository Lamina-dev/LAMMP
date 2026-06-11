/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../../include/lammp/lmmp.h"
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>


static lmmp_abort_fn lmmp_abort_func = NULL;
static mtx_t abort_func_mtx;
static once_flag abort_func_init_flag = ONCE_FLAG_INIT;

static void init_abort_lock(void) { 
    mtx_init(&abort_func_mtx, mtx_plain); 
}

lmmp_abort_fn lmmp_set_abort_fn(lmmp_abort_fn func) {
    call_once(&abort_func_init_flag, init_abort_lock);

    mtx_lock(&abort_func_mtx);
    lmmp_abort_fn old = lmmp_abort_func;
    lmmp_abort_func = func;
    mtx_unlock(&abort_func_mtx);

    return old;
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
    call_once(&abort_func_init_flag, init_abort_lock);

    mtx_lock(&abort_func_mtx);
    lmmp_abort_fn fn = lmmp_abort_func;
    mtx_unlock(&abort_func_mtx);

    if (fn != NULL) {
        fn(type, msg, func, line);
    } else {
        fprintf(stderr, "LAMMP abort at [%s]:%d\n", func, line);
        fprintf(stderr, "Abort type: %s, abort msg: \n%s\n", type_to_str(type), msg);
        fflush(stderr);
        abort();
    }
}