/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of mylgpllib, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../include/test_short.hpp"

void test_powmod() {
    ulong a = 9213;
    ulong b = 0;
    ulong m = 81007;
    ulong res = lmmp_powmod_ulong_(a, b, m);
    std::cout << "a^b mod m = " << res << std::endl;
}