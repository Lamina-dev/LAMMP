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

#include "../../../include/lammp/inter_buffer.hpp"
#include "../../../include/lammp/lammp.hpp"
#include "../../../include/lammp/mont64_div64.hpp"
#include "../../../include/lammp/uint128.hpp"
#include "../../../include/lammp/uint192.hpp"
#include "../../../include/lammp/base_cal.hpp"

bool is_writable(const std::filesystem::path& p);

// 生成固定长度的随机整数向量
lammp::_internal_buffer<0> generateRandomIntVector(size_t length, uint64_t min_val = 0, uint64_t max_val = UINT64_MAX);
std::vector<uint64_t> generateRandomIntVector_(size_t length, uint64_t min_val = 0, uint64_t max_val = UINT64_MAX);

long long bench_add(int len1, int len2);
long long bench_sub(int len1, int len2);
long long bench_mul(int len1, int len2);
long long bench_sqr(int len1);
long long bench_div(int len1, int len2);
long long bench_barrett_pre_div(int N, int len);
long long bench_num2binary(int len);
long long bench_binary2num(int len);
void run_benchmarks_and_save(const std::string& filename, const std::vector<int>& lengths, int repetitions = 5);
void bench_mul_balance();
void bench_div_128();
void bench_mul_128();
void bench_mul_192();
void bench_div_128_support();
inline void mul64x64to128_buildin(uint64_t a, uint64_t b, uint64_t& low, uint64_t& high);

void bench_barrett_2powN();
void bench_knuth_div();

#endif  // __BENCHMARK_HPP__
