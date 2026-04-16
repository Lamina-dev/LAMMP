/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include <chrono>

#include "../include/test_short.hpp"

void test_remove() {
    mp_limb_t base[3] = {2831, 2223330, 0x2231};
#define xn 10
    mp_limb_t x[xn] = {29387743, 2873, 3748, 22383, 9387793022, 9378649, 29233009820, 230, 91820192, 322298330};
    ulong exp = 12223;

    mp_size_t bn = lmmp_pow_size_(base, 3, exp) + 1;
    mp_ptr b = (mp_ptr)lmmp_alloc(bn * sizeof(mp_limb_t));
    
    mp_size_t n2 = lmmp_pow_(b, bn, base, 3, exp);
    mp_ptr a = (mp_ptr)lmmp_alloc((n2 + xn) * sizeof(mp_limb_t));
    lmmp_mul_(a, b, n2, x, xn);
    n2 += xn;
    n2 -= a[n2 - 1] == 0;

    auto start = std::chrono::high_resolution_clock::now();
    mp_size_t n3 = lmmp_remove_(a, &n2, base, 3);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << duration.count() << " microseconds" << std::endl;
    
    if (n3 != exp) {
        std::cout << "n3 != exp" << std::endl;
        std::cout << "n3: " << n3 << std::endl;
        std::cout << "exp: " << exp << std::endl;
        goto end;
    }
    if (n2 != xn) {
        std::cout << "n2 != xn" << std::endl;
        goto end;
    }
    for (ulong i = 0; i < xn; i++) {
        if (a[i] != x[i]) {
            std::cout << "a[" << i << "] != x[" << i << "]" << std::endl;
            goto end;
        }
    }
    std::cout << "n3: " << n3 << std::endl;
end:
    lmmp_free(b);
    lmmp_free(a);
    lmmp_leak_tracker;
}