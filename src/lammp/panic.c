#include "../../include/lammp/lmmp.h"
#include <stdio.h>
#include <stdlib.h>

static lmmp_abort_fn lmmp_abort_func = NULL;

lmmp_abort_fn lmmp_set_abort_fn(lmmp_abort_fn func) {
    lmmp_abort_fn old_func = lmmp_abort_func;
    lmmp_abort_func = func;
    return old_func;
}

#define type_to_str(type)                                                 \
      (type == LAMMP_ERROR_ASSERT_FAILURE)       ? "ASSERT_FAILURE"       \
    : (type == LAMMP_ERROR_DEBUG_ASSERT_FAILURE) ? "DEBUG_ASSERT_FAILURE" \
    : (type == LAMMP_ERROR_PARAM_ASSERT_FAILURE) ? "PARAM_ASSERT_FAILURE" \
    : (type == LAMMP_ERROR_MEMORY_ALLOC_FAILURE) ? "MEMORY_ALLOC_FAILURE" \
    : (type == LAMMP_ERROR_MEMORY_FREE_FAILURE)  ? "MEMORY_FREE_FAILURE"  \
    : (type == LAMMP_ERROR_OUT_OF_BOUNDS)        ? "OUT_OF_BOUNDS"        \
    : (type == LAMMP_ERROR_MEMORY_LEAK)          ? "MEMORY_LEAK"          \
    : (type == LAMMP_ERROR_UNEXPECTED_ERROR)     ? "UNEXPECTED_ERROR"     \
                                                 : "UNKNOWN_TYPE"

void lmmp_abort(lmmp_error_t type, const char* msg, const char* file, int line) {
    if (lmmp_abort_func != NULL) {
        lmmp_abort_func(type, msg, file, line);
    } else {
        fprintf(stderr, "%s:%d: LAMMP abort\n", file, line);
        fprintf(stderr, "abort type: %s, abort msg: \n%s\n", type_to_str(type), msg);
        fflush(stderr);
        abort();
    }
}