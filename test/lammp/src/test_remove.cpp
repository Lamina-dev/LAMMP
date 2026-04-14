/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include <chrono>

#include "../include/test_short.hpp"

void test_remove() {
    mp_limb_t base[3] = {2301, 0, 0xf123323};
#define xn 5
    mp_limb_t x[xn] = {9378649, 29233009820, 230, 91820192, 398330};
    ulong exp = 211;

    mp_size_t bn = lmmp_pow_size_(base, 3, exp) + 1;
    mp_ptr b = (mp_ptr)lmmp_alloc(bn * sizeof(mp_limb_t));
    
    mp_size_t n2 = lmmp_pow_(b, bn, base, 3, exp);
    mp_ptr a = (mp_ptr)lmmp_alloc((n2 + xn) * sizeof(mp_limb_t));
    lmmp_mul_(a, b, n2, x, xn);
    n2 += xn;
    n2 -= a[n2 - 1] == 0;

    auto start = std::chrono::high_resolution_clock::now();
    mp_size_t n3 = lmmp_remove_(a, &n2, x, xn - 1);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << duration.count() << " microseconds" << std::endl;
    for (mp_size_t i = 0; i < n2; i++) {
        std::cout << a[i] << "\n";
    }
    std::cout << std::endl;
    std::cout << "n3: " << n3 << std::endl;

    lmmp_free(b);
}