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

void test_trialdiv() {
    size_t len = 400, n = 62535;
    mp_ptr a = ALLOC_TYPE(len, mp_limb_t);
    a[0] = 10343298;
    a[1] = 12345678;
    a[2] = 8573937462848;
    a[32] = 84934568489;
    a[len - 1] = 98203933384424;
    auto start = std::chrono::high_resolution_clock::now();
    ushort bn = 0;
    ushortp b = lmmp_trialdiv_(a, 389, n, &bn);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "Time elapsed: " << duration << " microseconds" << std::endl;

    for (size_t i = 0; i < bn; i++) {
        std::cout << "Factor " << ": " << b[i] << "\n";
    }

    lmmp_free(a);
    lmmp_free(b);
}