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

#include "../include/test_short.hpp"
#include <iostream>
#include <chrono>

#define ALLOC_TYPE(n, type) (type*)lmmp_alloc((n) * sizeof(type))


void test_sqrt_1() {
    mp_limb_t a0 = 0x22212ffffffff;
    mp_limb_t b[2];
    lmmp_sqr_basecase_(b, &a0, 1);
    lmmp_dec(b);

    auto start1 = std::chrono::high_resolution_clock::now();
    for (mp_ssize_t i = 0; i < 1000000; ++i) {
        lmmp_sqrt_2_(b[0], b[1]);
        b[1] *= b[0] + 198218233;
        b[0] += 11122222333;
    }
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();
    std::cout << duration1 << "\n";


    // std::cout << "a0 = " << std::hex << a0 << "\n";
    // std::cout << "a  = " << std::hex << a << "\n";
}

void test_sqrt_4() {
    mp_limb_t a0[2] = {0xffffffffffffffff, 0xa311ffffffffffff};
    mp_limb_t b[4];
    //lmmp_fill(b, 0, 4, LIMB_MAX);
    lmmp_sqr_basecase_(b, a0, 2);
    lmmp_dec(b);

    //mp_size_t bn = (b[3] > 0) ? 4 : 3;

    auto start1 = std::chrono::high_resolution_clock::now();
    for (mp_ssize_t i = 0; i < 1000000; ++i) {
        mp_limb_t a[2];
        lmmp_sqrt_4_(a, b, 4);
        b[2] ^= a[0];
        b[3] |= b[0] + 1002932293843;
        //b[3] |= 0x4000000000000000;
        b[0] += a[1];
    }
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();
    std::cout << duration1 << "\n";


    //std::cout << "a0 = " << std::hex << a0[1] << " " << a0[0] << "\n";
    //std::cout << "a  = " << std::hex << a[1] << " " << a[0] << "\n";
}

void test_sqrt() {
    mp_size_t len = 41123, ni = 0;
    mp_ptr a0 = ALLOC_TYPE(len, mp_limb_t);
    mp_ptr b = ALLOC_TYPE(2 * len, mp_limb_t);

    //lmmp_fill(a0, 0, len, LIMB_MAX);
    len = lmmp_seed_random_(a0, len, 221, 0);

    auto start1 = std::chrono::high_resolution_clock::now();
    lmmp_sqr_(b, a0, len);
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();
    std::cout << duration1 << "\n";

    lmmp_dec(b);
    mp_size_t bn = 2 * len;
    bn -= b[bn - 1] == 0 ? 1 : 0;

    //mp_size_t bn = 2 * len;
    //lmmp_fill(b, 0, bn, LIMB_MAX);

    mp_size_t an = ni + bn / 2;
    mp_ptr a = ALLOC_TYPE(an, mp_limb_t);

    auto start2 = std::chrono::high_resolution_clock::now();
    an = lmmp_sqrtapprox_(a, b, bn, ni);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();
    std::cout << duration2 << "\n";

    if (an != len) {
        std::cout << "an != bn\n";
        goto end;
    }

    for (mp_ssize_t i = len - 1; i >= 0; --i) {
        if (a0[i] != a[i]) {
            std::cout << "i = " << i << "\n";
            std::cout << "a0 = " << a0[i] << "\n";
            std::cout << "a  = " << a[i] << "\n";
            goto end;
        }
    }

end:
    lmmp_free(a0);
    lmmp_free(a);
    lmmp_free(b);
}