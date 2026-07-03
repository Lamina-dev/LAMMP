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

void test_sqrt() {
    mp_limb_t a[2] = { 0ull, 0x8000000000000000ull };
    mp_limb_t b[2] = { 0ull, 0x0ull };
    mp_limb_t c[2] = { 0ull, 0x0ull };
    lmmp_sqrt_(b, c, a, 2, 0);
    std::cout << "b = " << std::hex << b[1] << std::hex << b[0] << std::endl;
    std::cout << "c = " << std::hex << c[1] << std::hex << c[0] << std::endl;
}