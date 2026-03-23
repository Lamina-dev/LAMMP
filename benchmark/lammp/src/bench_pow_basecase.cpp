/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of mylgpllib, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../include/benchmark.hpp"

long long bench_pow(int len1, int len2) {
    mp_ptr a = ALLOC_TYPE(len1, mp_limb_t);
    ger_random_numbers(len1, a);

    mp_size_t bn = lmmp_pow_size_(a, len1, len2);
    mp_ptr b = ALLOC_TYPE(bn, mp_limb_t);
    
    auto start = std::chrono::high_resolution_clock::now();
    bn = lmmp_pow_(b, bn, a, len1, len2);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    lmmp_free(a);
    lmmp_free(b);
    return duration.count();
}