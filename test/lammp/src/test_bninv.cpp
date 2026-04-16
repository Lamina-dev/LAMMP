/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../include/test_short.hpp"
#include <chrono>

#define ALLOC_TYPE(n, type) (type*)lmmp_alloc((n) * sizeof(type))

void test_bninv() {
    mp_limb_t na = 234233;
    mp_limb_t ni = 2132;

    mp_ptr numa = (mp_ptr)lmmp_alloc(na * sizeof(mp_limb_t));
    mp_ptr dstq1 = ALLOC_TYPE(na + ni + 2, mp_limb_t);
    mp_ptr dstq2 = ALLOC_TYPE(na + ni + 2, mp_limb_t);
    lmmp_seed_random_(numa, na - 1, 23923, 0);
    numa[na - 1] = (1ull << 63) + 1292210832131;

    auto start = std::chrono::high_resolution_clock::now();
    mp_ptr B = ALLOC_TYPE(2 * na + ni + 1, mp_limb_t);
    lmmp_zero(B, 2 * na + ni + 1);
    B[2 * na + ni] = 1;
    lmmp_div_(dstq1, NULL, B, 2 * na + ni + 1, numa, na);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Time elapsed: " << duration.count() << " microseconds" << std::endl;

    auto start2 = std::chrono::high_resolution_clock::now();
    lmmp_bninv_(dstq2, numa, na, ni);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2);
    std::cout << "Time elapsed: " << duration2.count() << " microseconds" << std::endl;

    int cnt = 0;
    for (mp_size_t i = 0; i < na + ni + 2; i++) {
        if (dstq1[i] != dstq2[i]) {
            std::cout << "dstq1[" << i << "] = " << dstq1[i] << " dstq2[" << i << "] = " << dstq2[i] << std::endl;
            cnt++;
        }
    }
    std::cout << "cnt: " << cnt << std::endl;

    lmmp_free(numa);
    lmmp_free(dstq1);
    lmmp_free(dstq2);
    lmmp_free(B);
    lmmp_leak_tracker;
}
