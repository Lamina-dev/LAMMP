/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include <chrono>

#include "../include/test_short.hpp"


#define ALLOC_TYPE(n, type) (type*)lmmp_alloc((n) * sizeof(type))

void test_binvert() {
    mp_limb_t na = 32545;

    mp_ptr numa = ALLOC_TYPE(na, mp_limb_t);
    mp_ptr tp = ALLOC_TYPE(5 * (na + 1) / 2, mp_limb_t);
    mp_ptr dst = ALLOC_TYPE(na, mp_limb_t);
    mp_ptr one = ALLOC_TYPE(na, mp_limb_t);
    lmmp_seed_random_(numa, na, 22332923, 0);
    numa[0] |= 1;


    auto start = std::chrono::high_resolution_clock::now();
    lmmp_binvert_n_dc_(dst, numa, na, tp);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Time elapsed: " << duration.count() << " microseconds" << std::endl;

    auto start2 = std::chrono::high_resolution_clock::now();
    lmmp_mullo_(one, dst, numa, na);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2);
    std::cout << "Time elapsed: " << duration2.count() << " microseconds" << std::endl;

    if (one[0] == 1) {
        for (size_t i = 1; i < na; i++) {
            if (one[i] != 0) {
                std::cout << "i = " << i << "\n";
                std::cout << "failed\n";
                goto end;
            }
        }
    }

    end:
    lmmp_free(numa);
    lmmp_free(dst);
    lmmp_free(one);
    lmmp_free(tp);
    lmmp_leak_tracker;
}
