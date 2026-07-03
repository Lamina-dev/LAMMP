/**
 *  Copyright (C) 2026 HJimmyK(Jericho Knox)
 *
 *  This file is part of LAMMP.
 *
 *  LAMMP is free software: you can redistribute it and/or modify it under
 *  the terms of the GNU Lesser General Public License (LGPL) as published
 *   by the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed WITHOUT ANY WARRANTY.
 *
 *  See <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>

#include "../../include/lammp/lmmpn.h"
#include "../../include/lammp/mprand.h"

#define ALLOC_TYPE(n, type) (type*)lmmp_alloc((n) * sizeof(type))

int main() {
    lmmp_global_init();

    mp_size_t n = 1000;
    mp_ptr a = ALLOC_TYPE(n, mp_limb_t);
    lmmp_zero(a, n);
    a[n - 1] = 1;
    a[3] = 2;
    a[4] = 123;
    mp_size_t str_len = lmmp_to_str_len_(a, n, 101);
    mp_byte_t* str = ALLOC_TYPE(str_len, mp_byte_t);

    str_len = lmmp_to_str_(str, a, n, 101);

    mp_size_t limb_len = lmmp_from_str_len_(str, str_len, 101);

    mp_ptr b = ALLOC_TYPE(limb_len, mp_limb_t);
    limb_len = lmmp_from_str_(b, str, str_len, 101);

    for (mp_size_t i = 0; i < limb_len; ++i) {
        std::cout << b[i] << " ";
    }
    std::cout << std::endl;

    lmmp_free(a);
    lmmp_free(str);
    lmmp_free(b);
    lmmp_global_deinit();
    return 0;
}