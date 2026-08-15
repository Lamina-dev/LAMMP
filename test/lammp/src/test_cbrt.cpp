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

#include <chrono>

#include "../include/test_short.hpp"

#define ALLOC_TYPE(n, type) (type*)lmmp_alloc((n) * sizeof(type))

static void lmmp_cube_(mp_ptr dst, mp_srcptr numa, mp_size_t len) {
    mp_ptr tp = ALLOC_TYPE(2 * len, mp_limb_t);

    lmmp_sqr_(tp, numa, len);
    lmmp_mul_(dst, tp, 2 * len, numa, len);
    lmmp_free(tp);
}

void test_cbrt() {
    mp_size_t len = 100000 + 100;
    mp_ptr a0 = ALLOC_TYPE(len, mp_limb_t);
    mp_ptr b = ALLOC_TYPE(3 * len, mp_limb_t);
    mp_ptr tp = ALLOC_TYPE(4 * len + 1, mp_limb_t);

    //lmmp_fill(a0, 0, len, LIMB_MAX);
    lmmp_random_(a0 + 100000, 100);
    lmmp_fill(a0, 0, len, 0);
    a0[len - 1] = 0xeeffffffffffffff;

    auto start1 = std::chrono::high_resolution_clock::now();
    lmmp_cube_(b, a0, len);
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();
    std::cout << duration1 << "\n";

    lmmp_dec(b);
    mp_ptr a = ALLOC_TYPE(len, mp_limb_t);

    /*
    std::cout << "input = ";
    for (mp_ssize_t i = 3 * len - 1; i >= 0; --i) {
        printf("%016llx", b[i]);
    }
    std::cout << "\n";
    */
    
    auto start2 = std::chrono::high_resolution_clock::now();
    lmmp_cbrt_divide_(a, b, len, tp, 1);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();
    std::cout << duration2 << "\n";

    /*
    std::cout << "cbrt = ";
    for (mp_ssize_t i = len - 1; i >= 0; --i) {
        printf("%016llx", a[i]);
    }
    std::cout << "\n";

    std::cout << "rem = ";
    for (mp_ssize_t i = 2 * len; i >= 0; --i) {
        printf("%016llx", b[i]);
    }
    std::cout << "\n";

end:
*/
    lmmp_free(a0);
    lmmp_free(a);
    lmmp_free(b);
    lmmp_free(tp);
}
