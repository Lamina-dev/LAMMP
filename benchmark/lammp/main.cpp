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

#include <fstream>  
#include "include/benchmark.hpp"
#include <iostream>
#include <chrono>


int main() {
    lmmp_global_init();
    std::cout << bench_factorial(10000000) << std::endl;
    lmmp_global_deinit();
    return 0;
}