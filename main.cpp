#include <vector>
#include <iostream>
#include <chrono>
#include <random>
#include <cmath>
#include <fstream>
#include "src/hint.hpp"

// 生成固定长度的随机整数向量
std::vector<uint64_t> generateRandomIntVector(size_t length, uint64_t min_val = 0, uint64_t max_val = UINT64_MAX)
{
	std::vector<uint64_t> vec;
	vec.reserve(length);

	unsigned seed = 120;
	std::default_random_engine generator(seed);
	std::uniform_int_distribution<uint64_t> distribution(min_val, max_val);

	for (int i = 0; i < length; ++i)
	{
		vec.push_back(distribution(generator));
	}

	return vec;
}

double test_time(size_t bits)
{
	using namespace HyperInt::Arithmetic::Numeral;
	const size_t base = 10;
	const double base_d = BaseTable::table2[base - 2];
	const uint64_t base_num = BaseTable::table1[base - 2][0];

	using namespace HyperInt::Arithmetic;
	size_t len = (1ull << bits);
	std::vector<uint64_t> a(len, UINT64_MAX);
	static std::mt19937 gen(1);
	std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);
	for (auto &&i : a)
	{
		i = dist(gen);
	}

	std::vector<uint64_t> c(get_buffer_size(len, base_d), 0);
	auto t1 = std::chrono::steady_clock::now();
	size_t len2 = num2base(a.data(), len, base, c.data());
	auto t2 = std::chrono::steady_clock::now();

	std::chrono::duration<double> time_span = t2 - t1;
	return time_span.count();
}

auto test_time_len(size_t len)
{
	using namespace HyperInt::Arithmetic::Numeral;
	const size_t base = 10;
	const double base_d = BaseTable::table2[base - 2];
	const uint64_t base_num = BaseTable::table1[base - 2][0];

	using namespace HyperInt::Arithmetic;
	std::vector<uint64_t> a(len, UINT64_MAX);
	static std::mt19937 gen(1);
	std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);
	for (auto &&i : a)
	{
		i = dist(gen);
	}

	std::vector<uint64_t> c(get_buffer_size(len, base_d), 0);
	auto t1 = std::chrono::steady_clock::now();
	size_t len2 = num2base(a.data(), len, base, c.data());
	auto t2 = std::chrono::steady_clock::now();

	return std::chrono::duration<double>(t2 - t1).count();
}

void test1()
{
	using namespace HyperInt::Arithmetic::Numeral;

	size_t len = 123;
	const double base_d = BaseTable::table2[8];
	const uint64_t base_num = BaseTable::table1[8][0];
	size_t len_new = std::ceil(base_d * len) + 1;

	std::vector<uint64_t> vec1 = generateRandomIntVector(len);
	std::vector<uint64_t> vec2(len, 0);
	std::copy(vec1.begin(), vec1.end(), vec2.begin());

	std::vector<uint64_t> res2(len_new, 0);
	std::vector<uint64_t> res1(len_new, 0);

	size_t len1 = num2base_classic(vec1.data(), len, base_num, res1.data());
	std::cout << "len1: " << len1 << std::endl;
	for (size_t i = len1 - 20; i < len1; i++)
	{
		std::cout << res1[i] << " ";
	}
	std::cout << std::endl
			  << std::endl;

	auto t1 = std::chrono::steady_clock::now();
	size_t len2 = num2base(vec2.data(), len, 10, res2.data());
	auto t2 = std::chrono::steady_clock::now();
	for (size_t i = len2 - 20; i < len2; i++)
	{
		std::cout << res2[i] << " ";
	}
	std::cout << std::endl
			  << std::endl;
	std::chrono::duration<double> time_span = t2 - t1;
	std::cout << "len2: " << len2 << std::endl;
	std::cout << "time: " << time_span.count() << std::endl;
}

void test2()
{
	using namespace HyperInt::Arithmetic::Numeral;
	const double base_d = BaseTable::table2[8];
	const uint64_t base_num = BaseTable::table1[8][0];
	size_t max_len = 1ull << 8;
	_2pow64_index_list index_list = create_2pow64_index_list(max_len, base_num, base_d);
	_2pow64_index_list current = index_list;

	while (current != nullptr)
	{
		std::cout << "length: " << current->length << std::endl;
		std::cout << "index: " << current->index << std::endl;
		for (size_t i = 0; i < current->length; i++)
		{
			std::cout << current->base_index[i] << " ";
		}
		std::cout << std::endl
				  << std::endl;
		current = current->back;
	}
}

void test3()
{
	using namespace HyperInt::Arithmetic::Numeral;
	const double base_d = BaseTable::table2[8];
	const uint64_t base_num = BaseTable::table1[8][0];
	// 161
	size_t index = 1269;
	size_t len = get_buffer_size(index + 1, base_d);
	std::vector<uint64_t> vec(len, 0);

	size_t vec_len = base_power_index(base_num, index, vec.data(), len);

	std::cout << "vec_len: " << vec_len << std::endl;
	for (size_t i = 0; i < vec_len; i++)
	{
		std::cout << vec[i] << " ";
	}
	std::cout << std::endl;

	size_t res_len = get_buffer_size(vec_len, base_d) + 1;
	std::vector<uint64_t> res(res_len, 0);
	res_len = num2base(vec.data(), vec_len, base_num, res.data());
	std::cout << "res_len: " << res_len << std::endl;
	for (size_t i = 0; i < res_len; i++)	
	{
		std::cout << res[i] << " ";
	}
	std::cout << std::endl;
}

void test4(){
	using namespace HyperInt;
	uint64_t base = 10000000000000000000ull;
	uint64_t a = base - 1;
	uint64_t b = base - 1;
	bool carry = false;
	uint64_t d = 0;
	uint64_t c = add_carry_base(a, b, carry, base);

	std::cout << c << std::endl;
	std::cout << d << std::endl;
	std::cout << carry << std::endl;
}


int main()
{
	test3();
	return 0;
}
