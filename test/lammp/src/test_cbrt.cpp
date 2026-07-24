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

void test_cbrt_6() {
    mp_size_t len = 7;
    mp_ptr b = ALLOC_TYPE(len, mp_limb_t);
    mp_limb_t a0[2];
    mp_limb_t a[2];
    a0[0] = 0x123456789abcdef;
    a0[1] = 0x323456789abcdef;
    len = lmmp_pow_(b, len, a0, 2, 3);

    b[0]--;
    auto start1 = std::chrono::high_resolution_clock::now();
    lmmp_cbrt_6_(a, b, len);
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();
    std::cout << duration1 << "\n";

    std::cout << "a0 = " << std::hex << a0[1] << " " << a0[0] << "\n";
    std::cout << "a  = " << std::hex << a[1] << " " << a[0] << "\n";
}


void test_cbrt() {
    mp_size_t len = 19, ni = 0;
    mp_ptr a0 = ALLOC_TYPE(len, mp_limb_t);
    mp_ptr b = ALLOC_TYPE(3 * len, mp_limb_t);
    mp_ptr tp = ALLOC_TYPE(2 * len, mp_limb_t);

    len = lmmp_random_(a0, len);

    auto start1 = std::chrono::high_resolution_clock::now();
    mp_size_t bn = lmmp_cube_(b, a0, len, tp);
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();
    std::cout << duration1 << "\n";

    mp_ptr a = ALLOC_TYPE((bn + ni + 2) / 3 + 2, mp_limb_t);

    auto start2 = std::chrono::high_resolution_clock::now();
    mp_size_t an = lmmp_cbrtapprox_(a, b, bn, ni);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();
    std::cout << duration2 << "\n";

    std::cout << "len = " << len << "\n";
    std::cout << "an  = " << an << "\n";

    for (mp_ssize_t i = len - 1; i >= 0; --i) {
        if (a0[i] != a[i]) {
            std::cout << "a0 = " << std::hex << a0[i - 1] << "\n";
            std::cout << "a  = " << std::hex << a[i - 1] << "\n";
            break;
        }
    }

    lmmp_free(a0);
    lmmp_free(a);
    lmmp_free(b);
    lmmp_free(tp);
}