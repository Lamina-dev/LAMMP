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
#include <vector>
#include "../include/test_short.hpp"


void test_gcd2() {
    mp_limb_t a[2] = {52516872761, 1110};
    mp_limb_t b[2] = {52516872761, 1110};
    mp_limb_t r1[2];
    mp_limb_t r2[2] = {0, 0};
    lmmp_gcd_22_(r1, a, b);
    mp_size_t n = lmmp_gcd_lehmer_(r2, a, 2, b, 2);
    std::cout << r1[0] << " " << r1[1] << std::endl;
    std::cout << r2[0] << " " << r2[1] << std::endl;
    std::cout << n << std::endl;
    return;
}