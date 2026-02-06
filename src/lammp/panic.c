#include "../../include/lammp/lmmp.h"
#include <stdio.h>
#include <stdlib.h>

static lmmp_abort_func_t lmmp_abort_func = NULL;

int lmmp_set_abort_func(lmmp_abort_func_t func) {
    if (lmmp_abort_func == NULL) {
        lmmp_abort_func = func;
        return 0;
    } else {
        lmmp_abort_func = func;
        return 1;
    }
}

void lmmp_abort(int type, const char* msg, const char* file, int line) {
    if (lmmp_abort_func != NULL) {
        lmmp_abort_func(type, msg, file, line);
    } else {
        fprintf(stderr, "%s:%d: LAMMP abort\n", file, line);
        fprintf(stderr, "abort type: %d, abort msg: \n%s\n", type, msg);
        fflush(stderr);
        abort();
    }
}