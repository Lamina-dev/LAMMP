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

#ifndef __LAMMP_BENCHMARK_HPP__
#define __LAMMP_BENCHMARK_HPP__

#include <chrono>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "../../../include/lammp/lmmpn.h"
#include "../../../include/lammp/numth.h"
#include "../../../include/lammp/mprand.h"

void ger_random_numbers(int len, mp_ptr dst, mp_size_t min = 0, mp_size_t max = LIMB_MAX);

long long bench_add(int len1, int len2, int n);
long long bench_sub(int len1, int len2);
long long bench_mul(int len1, int len2);
long long bench_div(int len1, int len2);
long long bench_pow(int len1, int len2);
long long bench_pow_win2(int len1, int len2);
long long bench_factorial(unsigned len1);
long long bench_powmod_ulong(int len);
long long bench_hash(int len, int type);
void bench_gcd2();

#endif  // __LAMMP_BENCHMARK_HPP__
