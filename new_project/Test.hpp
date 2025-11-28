#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "src/lammp.hpp"

bool is_writable(const std::filesystem::path& p);

// 生成固定长度的随机整数向量
lammp::_internal_buffer<0> generateRandomIntVector(size_t length, uint64_t min_val = 0, uint64_t max_val = UINT64_MAX);
std::vector<uint64_t> generateRandomIntVector_(size_t length, uint64_t min_val = 0, uint64_t max_val = UINT64_MAX);

double test_add(int len1, int len2);
double test_sub(int len1, int len2);
double test_mul(int len1, int len2);
double test_div(int len1, int len2);
double test_num2binary(int len);
double test_binary2num(int len);
void run_tests_and_save(const std::string& filename, const std::vector<int>& lengths, int repetitions = 5);
void test_mul_balance();
void test_div_128();
void test_mul_128();
void test_mul_192();
inline void mul64x64to128_buildin(uint64_t a, uint64_t b, uint64_t& low, uint64_t& high);
void test_mod_div();

void test_barrett_2powN_div_num();

void test_knuth_div();

