/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>

#include "../../include/lammp/lmmpn.h"
#include "../../include/lammp/mprand.h"
int main() {
    TEMP_DECL;
    mp_size_t n = 1000;
    mp_ptr a = BALLOC_TYPE(n, mp_limb_t);
    lmmp_zero(a, n);
    a[n - 1] = 1;
    a[3] = 2;
    a[4] = 123;
    mp_size_t str_len = lmmp_to_str_len_(a, n, 101);
    mp_byte_t* str = BALLOC_TYPE(str_len, mp_byte_t);

    str_len = lmmp_to_str_(str, a, n, 101);

    mp_size_t limb_len = lmmp_from_str_len_(str, str_len, 101);

    mp_ptr b = BALLOC_TYPE(limb_len, mp_limb_t);
    limb_len = lmmp_from_str_(b, str, str_len, 101);

    for (mp_size_t i = 0; i < limb_len; ++i) {
        std::cout << b[i] << " ";
    }
    std::cout << std::endl;

    TEMP_FREE;
    return 0;
}