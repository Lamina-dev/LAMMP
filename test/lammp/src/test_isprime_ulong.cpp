/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of mylgpllib, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../include/test_short.hpp"

void test_isprime_ulong() {
    ulong n = 1ull;
    if (lmmp_is_prime_ulong_(n)) {
        std::cout << n << " is a prime number" << std::endl;
    }
    else {
        std::cout << n << " is not a prime number" << std::endl;
    }
}