#ifndef __BENCHMARK_HPP__
#define __BENCHMARK_HPP__

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

bool is_writable(const std::filesystem::path& p);

void ger_random_numbers(int len, mp_ptr dst, mp_size_t min = 0, mp_size_t max = LIMB_MAX);

long long bench_add(int len1, int len2);
long long bench_sub(int len1, int len2);
long long bench_mul(int len1, int len2);
long long bench_sqr(int len1);
long long bench_div(int len1, int len2);
long long bench_pow(int len1, int len2);
long long bench_factorial(unsigned len1);
long long bench_powmod_ulong(int len);
void bench_div_128();


#endif  // __BENCHMARK_HPP__
