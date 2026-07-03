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
#include <random>

#include "../include/test_short.hpp"

#define ALLOC_TYPE(n, type) (type*)lmmp_alloc((n) * sizeof(type))

void test_rand() {

    uint64_t seed = 883574;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<mp_limb_t> dis(0, UINT64_MAX);
    mp_size_t len = 100001;
    mp_ptr dst = ALLOC_TYPE(len, mp_limb_t);
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < len; i++) {
        dst[i] = dis(gen);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "mt19937      duration: " << duration << " microseconds" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    lmmp_seed_random_(dst, len, seed, 0);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "pcg64-128    duration: " << duration << " microseconds" << std::endl;
    
    start = std::chrono::high_resolution_clock::now();
    lmmp_seed_random_(dst, len, seed, 1);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "xoshiro256++ duration: " << duration << " microseconds" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    lmmp_strong_rng_t* rng = lmmp_strong_rng_init_(len, seed);
    lmmp_strong_random_(dst, len, rng);
    lmmp_strong_rng_free_(rng);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "strong_rng   duration: " << duration << " microseconds" << std::endl;

    lmmp_free(dst);

    lmmp_global_rng_init_(0, 1);
}