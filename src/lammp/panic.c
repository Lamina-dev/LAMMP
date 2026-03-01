#include "../../include/lammp/lmmp.h"
#include <stdio.h>
#include <stdlib.h>

static lmmp_abort_fn lmmp_abort_func = NULL;

lmmp_abort_fn lmmp_set_abort_fn(lmmp_abort_fn func) {
    lmmp_abort_fn old_func = lmmp_abort_func;
    lmmp_abort_func = func;
    return old_func;
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

void lmmp_abort(lmmp_error_t type, const char* msg, const char* file, int line) {
    if (lmmp_abort_func != NULL) {
        lmmp_abort_func(type, msg, file, line);
    } else {
        fprintf(stderr, "LAMMP abort at %s:%d\n", file, line);
        fprintf(stderr, "Abort type: %s, abort msg: \n%s\n", type_to_str(type), msg);
        fflush(stderr);
        abort();
    }
}