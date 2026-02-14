#include "../../include/lammp/lmmp.h"
#include <stdio.h>
#include <stdlib.h>

static lmmp_abort_func_t lmmp_abort_func = NULL;

lmmp_abort_func_t lmmp_set_abort_func(lmmp_abort_func_t func) {
    lmmp_abort_func_t old_func = lmmp_abort_func;
    lmmp_abort_func = func;
    return old_func;
}

#define type_to_str(type)                                                 \
      (type == LAMMP_ASSERT_FAILURE)       ? "LAMMP_ASSERT_FAILURE"       \
    : (type == LAMMP_DEBUG_ASSERT_FAILURE) ? "LAMMP_DEBUG_ASSERT_FAILURE" \
    : (type == LAMMP_MEMORY_ALLOC_FAILURE) ? "LAMMP_MEMORY_ALLOC_FAILURE" \
    : (type == LAMMP_OUT_OF_BOUNDS)        ? "LAMMP_OUT_OF_BOUNDS"        \
    : (type == LAMMP_UNEXPECTED_ERROR)     ? "LAMMP_UNEXPECTED_ERROR"     \
                                           : "LAMMP_UNKNOWN_TYPE"

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