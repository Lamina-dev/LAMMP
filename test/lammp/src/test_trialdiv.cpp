/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
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