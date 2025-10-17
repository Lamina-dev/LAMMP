#include <vector>
#include <iostream>
#include <chrono>
#include <random>
#include <cmath>
#include <fstream>
#include "Test.hpp"
/*
文件名：hint.hpp

文件名：BigFrac.hpp
包含：BigInt.hpp

文件名：BigInt.hpp

文件名：BigInt.cpp
包含：BigInt.hpp hint.hpp

文件名：BigFrac.cpp
包含：BigFrac.hpp 

*/



// double test_time(size_t bits)
// {
// 	using namespace HyperInt::Arithmetic::Numeral;
// 	const size_t base = 10;
// 	const double base_d = BaseTable::table2[base - 2];
// 	const uint64_t base_num = BaseTable::table1[base - 2][0];

// 	using namespace HyperInt::Arithmetic;
// 	size_t len = (1ull << bits);
// 	std::vector<uint64_t> a(len, UINT64_MAX);
// 	static std::mt19937 gen(1);
// 	std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);
// 	for (auto &&i : a)
// 	{
// 		i = dist(gen);
// 	}

// 	std::vector<uint64_t> c(get_buffer_size(len, base_d), 0);
// 	auto t1 = std::chrono::steady_clock::now();
// 	size_t len2 = num2base(a.data(), len, base, c.data());
// 	auto t2 = std::chrono::steady_clock::now();

// 	std::chrono::duration<double> time_span = t2 - t1;
// 	return time_span.count();
// }

// auto test_time_len(size_t len)
// {
// 	using namespace HyperInt::Arithmetic::Numeral;
// 	const size_t base = 10;
// 	const double base_d = BaseTable::table2[base - 2];
// 	const uint64_t base_num = BaseTable::table1[base - 2][0];

// 	using namespace HyperInt::Arithmetic;
// 	std::vector<uint64_t> a(len, UINT64_MAX);
// 	static std::mt19937 gen(1);
// 	std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);
// 	for (auto &&i : a)
// 	{
// 		i = dist(gen);
// 	}

// 	std::vector<uint64_t> c(get_buffer_size(len, base_d), 0);
// 	auto t1 = std::chrono::steady_clock::now();
// 	size_t len2 = num2base(a.data(), len, base, c.data());
// 	auto t2 = std::chrono::steady_clock::now();

// 	return std::chrono::duration<double>(t2 - t1).count();
// }

// void test1()
// {
// 	using namespace HyperInt::Arithmetic::Numeral;

// 	size_t len = 123;
// 	const double base_d = BaseTable::table2[8];
// 	const uint64_t base_num = BaseTable::table1[8][0];
// 	size_t len_new = std::ceil(base_d * len) + 1;

// 	std::vector<uint64_t> vec1 = generateRandomIntVector(len);
// 	std::vector<uint64_t> vec2(len, 0);
// 	std::copy(vec1.begin(), vec1.end(), vec2.begin());

// 	std::vector<uint64_t> res2(len_new, 0);
// 	std::vector<uint64_t> res1(len_new, 0);

// 	size_t len1 = num2base_classic(vec1.data(), len, base_num, res1.data());
// 	std::cout << "len1: " << len1 << std::endl;
// 	for (size_t i = len1 - 20; i < len1; i++)
// 	{
// 		std::cout << res1[i] << " ";
// 	}
// 	std::cout << std::endl
// 			  << std::endl;

// 	auto t1 = std::chrono::steady_clock::now();
// 	size_t len2 = num2base(vec2.data(), len, 10, res2.data());
// 	auto t2 = std::chrono::steady_clock::now();
// 	for (size_t i = len2 - 20; i < len2; i++)
// 	{
// 		std::cout << res2[i] << " ";
// 	}
// 	std::cout << std::endl
// 			  << std::endl;
// 	std::chrono::duration<double> time_span = t2 - t1;
// 	std::cout << "len2: " << len2 << std::endl;
// 	std::cout << "time: " << time_span.count() << std::endl;
// }

// void test2()
// {
// 	using namespace HyperInt::Arithmetic::Numeral;
// 	const double base_d = BaseTable::table2[8];
// 	const uint64_t base_num = BaseTable::table1[8][0];
// 	size_t max_len = 1ull << 8;
// 	_2pow64_index_list index_list = create_2pow64_index_list(max_len, base_num, base_d);
// 	_2pow64_index_list current = index_list;

// 	while (current != nullptr)
// 	{
// 		std::cout << "length: " << current->length << std::endl;
// 		std::cout << "index: " << current->index << std::endl;
// 		for (size_t i = 0; i < current->length; i++)
// 		{
// 			std::cout << current->base_index[i] << " ";
// 		}
// 		std::cout << std::endl
// 				  << std::endl;
// 		current = current->back;
// 	}
// }

// void test3()
// {
// 	using namespace HyperInt::Arithmetic::Numeral;
// 	const double base_d = 1 / BaseTable::table2[8];
// 	const uint64_t base_num = BaseTable::table1[8][0];
// 	// 161
// 	size_t len = 19900;
// 	std::vector<uint64_t> vec = generateRandomIntVector(len, 0, base_num);
// 	std::vector<uint64_t> vec_copy(len, 0);
// 	std::copy(vec.data(), vec.data() + len, vec_copy.data());
// 	std::vector<uint64_t> _vec(get_buffer_size(len, base_d), 0);
// 	size_t _vec_len = base2num(vec_copy.data(), len, 10, _vec.data());

// 	std::cout << "len: " << len << std::endl;
// 	std::cout << std::endl;

// 	size_t res_len = get_buffer_size(_vec_len, 1 / base_d);
// 	std::vector<uint64_t> res(res_len, 0);
// 	res_len = num2base(_vec.data(), _vec_len, base_num, res.data());
// 	std::cout << "res_len: " << res_len << std::endl;
// 	for (size_t i = res_len - 30; i < res_len; i++)	
// 	{
// 		std::cout << "i :" << i << " " << res[i] << " " << vec[i] << std::endl;
// 	}
// 	std::cout << std::endl;
// }

// void test4(){
// 	using namespace HyperInt;
// 	uint64_t base = 10000000000000000000ull;
// 	uint64_t a = base - 1;
// 	uint64_t b = base - 1;
// 	bool carry = false;
// 	uint64_t d = 0;
// 	uint64_t c = add_carry_base(a, b, carry, base);

// 	std::cout << c << std::endl;
// 	std::cout << d << std::endl;
// 	std::cout << carry << std::endl;
// }



int	main()
{
	// test_factorial(2000000);
	
	// // 配置测试长度，可以根据需要调整
	// std::vector<int> test_lengths = {2000, 3000, 4000};
	// // std::vector<int> test_lengths = { 320000, 360000, 400000, 440000, 480000, 520000, 560000, 600000, 640000 };

	// //每个测试重复的次数，用于计算统计值
	// const int repetitions = 3;

	// const std::string csv_path = "test_results.csv";
	// // 运行测试并保存结果
	// run_tests_and_save( csv_path, test_lengths, repetitions);
	test_mul_balance();
	return 0;
}
