/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of mylgpllib, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../include/benchmark.hpp"

void ger_random_numbers(int len, mp_ptr dst, mp_size_t min, mp_size_t max) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<mp_limb_t> dis(min, max);
    for (int i = 0; i < len; i++) {
        dst[i] = dis(gen);
    }
}