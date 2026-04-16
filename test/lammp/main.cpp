/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include <iostream>
#include "include/test_long.hpp"
#include "include/test_short.hpp"
#include <vector>

#include "../../include/lammp/impl/prime_table.h"

int main() {
    lmmp_global_init();
    test_remove();
    lmmp_global_deinit();
    return 0;
}