/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include <string.h>
#include <algorithm>
#include <iostream>
#include <string>
#include <chrono>
#include "../../include/lammp/lmmpn.h"
#include "../../include/lammp/mprand.h"

int main() {
    lmmp_global_init();

    mp_size_t len1 = 50000, len2 = len1 * 500;
    mp_size_t len = len1 + len2;
    mp_ptr a = (mp_ptr)lmmp_alloc(len1 * sizeof(mp_limb_t));
    mp_ptr b = (mp_ptr)lmmp_alloc(len2 * sizeof(mp_limb_t));

    len1 = lmmp_random_(a, len1);
    len2 = lmmp_random_(b, len2);

    mp_ptr c = (mp_ptr)lmmp_alloc(len * sizeof(mp_limb_t));
    auto start = std::chrono::high_resolution_clock::now();
    lmmp_mul_fft_(c, a, len1, b, len2);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "FFT multiplication time: " << duration.count() << " microseconds" << std::endl;
    lmmp_free(a);
    lmmp_free(b);
    lmmp_free(c);
    lmmp_global_deinit();
    return 0;
}
