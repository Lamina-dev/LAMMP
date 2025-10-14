#include "src/hint.hpp"
#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <fstream>
#include <iomanip>
#include <cstdio>
#include <filesystem>

bool is_writable(const std::filesystem::path &p)
{
    std::filesystem::perms perm = std::filesystem::status(p).permissions();
    return (perm & std::filesystem::perms::owner_write) != std::filesystem::perms::none;
}

// 生成固定长度的随机整数向量
std::vector<uint64_t> generateRandomIntVector(size_t length, uint64_t min_val = 0, uint64_t max_val = UINT64_MAX)
{
    std::vector<uint64_t> vec;
    vec.reserve(length);

    unsigned seed = 120;
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<uint64_t> distribution(min_val, max_val);

    for (size_t i = 0; i < length; ++i)
    {
        vec.push_back(distribution(generator));
    }

    return vec;
}

double test_add(int len1, int len2)
{
    using namespace HyperInt;
    using namespace HyperInt::Arithmetic;

    std::vector<uint64_t> vec1 = generateRandomIntVector(get_add_len(len1, len2));
    std::vector<uint64_t> vec2 = generateRandomIntVector(len2);

    auto start = std::chrono::high_resolution_clock::now();
    abs_add_binary(vec1.data(), len1, vec2.data(), len2, vec1.data());
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count();
}

double test_sub(int len1, int len2)
{
    using namespace HyperInt;
    using namespace HyperInt::Arithmetic;

    std::vector<uint64_t> vec1 = generateRandomIntVector(get_sub_len(len1, len2));
    std::vector<uint64_t> vec2 = generateRandomIntVector(len2);

    auto start = std::chrono::high_resolution_clock::now();
    abs_sub_binary(vec1.data(), len1, vec2.data(), len2, vec1.data());
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count();
}

double test_mul(int len1, int len2)
{
    using namespace HyperInt;
    using namespace HyperInt::Arithmetic;

    std::vector<uint64_t> vec1 = generateRandomIntVector(len1);
    std::vector<uint64_t> vec2 = generateRandomIntVector(len2);
    std::vector<uint64_t> res(get_mul_len(len1, len2), 0);
    auto start = std::chrono::high_resolution_clock::now();
    abs_mul64(vec1.data(), len1, vec2.data(), len2, res.data());
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count();
}

double test_div(int len1, int len2)
{
    using namespace HyperInt;
    using namespace HyperInt::Arithmetic;

    std::vector<uint64_t> vec1 = generateRandomIntVector(len1);
    std::vector<uint64_t> vec2 = generateRandomIntVector(len2);
    std::vector<uint64_t> res(get_div_len(len1, len2), 0);
    // 确保除数不为零
    for (auto &val : vec2)
    {
        if (val == 0)
            val = 1;
    }

    auto start = std::chrono::high_resolution_clock::now();
    abs_div64(vec1.data(), len1, vec2.data(), len2, res.data());
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count();
}

double test_num2binary(int len)
{
    using namespace HyperInt;
    using namespace HyperInt::Arithmetic::Numeral;

    const double base_d = BaseTable::table2[8];
    const uint64_t base_num = BaseTable::table1[8][0];
    std::vector<uint64_t> vec = generateRandomIntVector(len, 0, base_num);
    std::vector<uint64_t> res(get_buffer_size(len, 1 / base_d), 0);
    auto start = std::chrono::high_resolution_clock::now();
    base2num(vec.data(), len, 10, res.data());
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count();
}

double test_binary2num(int len)
{
    using namespace HyperInt;
    using namespace HyperInt::Arithmetic::Numeral;
    std::vector<uint64_t> vec = generateRandomIntVector(len);
    const double base_d = BaseTable::table2[8];
    const uint64_t base_num = BaseTable::table1[8][0];
    std::vector<uint64_t> res(get_buffer_size(len, base_d), 0);
    auto start = std::chrono::high_resolution_clock::now();
    num2base(vec.data(), len, 10, res.data());
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count();
}

// 运行测试并将结果写入CSV文件
void run_tests_and_save(const std::string &filename,
                        const std::vector<int> &lengths,
                        int repetitions = 5)
{
    std::ofstream out(filename);
    if (!out.is_open())
    {
        std::cerr << "could't open file: " << filename << std::endl;
        // 检查目录是否存在
        std::filesystem::path dir = std::filesystem::path(filename).parent_path();
        if (!std::filesystem::exists(dir))
        {
            std::cerr << "no exist: " << dir << std::endl;
        }
        else if (!std::filesystem::is_directory(dir))
        {
            std::cerr << "not a directory: " << dir << std::endl;
        }
        else if (!is_writable(dir))
        {
            std::cerr << "no write permission: " << dir << std::endl;
        }
        return;
    }

    // 写入CSV头部
    out << "opreate,len1(64bits),len2(64bits),mean time(us),min time(us),max time(us),stddev(us)" << std::endl;

    // 测试加法
    for (int len1 : lengths)
    {
        for (int len2 : lengths)
        {
            std::vector<double> times(repetitions, 0);

            for (int i = 0; i < repetitions; ++i)
            {
                times[i] = test_add(len1, len2);
            }

            // 计算统计值
            double sum = 0, min = times[0], max = times[0];
            for (double t : times)
            {
                sum += t;
                if (t < min)
                    min = t;
                if (t > max)
                    max = t;
            }
            double avg = sum / repetitions;

            // 计算标准差
            double variance = 0;
            for (double t : times)
            {
                variance += (t - avg) * (t - avg);
            }
            double std_dev = std::sqrt(variance / repetitions);
            // 写入CSV
            out << "add," << len1 << "," << len2 << ","
                << std::fixed
                << avg << "," << min << "," << max << "," << std_dev << std::endl;
        }
        std::cout << "add testing complete, length1: " << len1 << std::endl;
    }

    // 测试减法
    for (int len1 : lengths)
    {
        for (int len2 : lengths)
        {
            std::vector<double> times(repetitions, 0);
            for (int i = 0; i < repetitions; ++i)
            {
                times[i] = test_sub(len1, len2);
            }

            // 计算统计值
            double sum = 0, min = times[0], max = times[0];
            for (double t : times)
            {
                sum += t;
                if (t < min)
                    min = t;
                if (t > max)
                    max = t;
            }
            double avg = sum / repetitions;

            // 计算标准差
            double variance = 0;
            for (double t : times)
            {
                variance += (t - avg) * (t - avg);
            }
            double std_dev = std::sqrt(variance / repetitions);

            // 写入CSV
            out << "sub," << len1 << "," << len2 << ","
                << std::fixed << std::setprecision(3)
                << avg << "," << min << "," << max << "," << std_dev << std::endl;
        }
        std::cout << "sub testing complete, length1: " << len1 << std::endl;
    }

    // 测试乘法
    for (int len1 : lengths)
    {
        for (int len2 : lengths)
        {
            std::vector<double> times(repetitions, 0);
            for (int i = 0; i < repetitions; ++i)
            {
                times[i] = test_mul(len1, len2);
            }

            // 计算统计值
            double sum = 0, min = times[0], max = times[0];
            for (double t : times)
            {
                sum += t;
                if (t < min)
                    min = t;
                if (t > max)
                    max = t;
            }
            double avg = sum / repetitions;

            // 计算标准差
            double variance = 0;
            for (double t : times)
            {
                variance += (t - avg) * (t - avg);
            }
            double std_dev = std::sqrt(variance / repetitions);

            // 写入CSV
            out << "mul," << len1 << "," << len2 << ","
                << std::fixed << std::setprecision(3)
                << avg << "," << min << "," << max << "," << std_dev << std::endl;
        }
        std::cout << "mul testing complete, length1: " << len1 << std::endl;
    }

    // 测试除法
    for (int len1 : lengths)
    { // 只选择小于 len1 的 len2 进行测试
        for (int len2 : lengths){
            if (len1 <= len2) // 跳过 len1 不大于 len2 的情况
                continue;
            std::vector<double> times(repetitions, 0);
            for (int i = 0; i < repetitions; ++i)
            {
                times[i] = test_div(len1, len2);
            }
            // 计算统计值
            double sum = 0, min = times [0], max = times [0];
            for (double t : times){
                sum += t;
                if (t < min)
                    min = t;
                if (t > max)
                max = t;
            }
            double avg = sum /repetitions;
            // 计算标准差
            double variance = 0;
            for (double t : times){
                variance += (t - avg) * (t - avg);
            }
            double std_dev = std::sqrt (variance /repetitions);
            // 写入 CSV
            out << "div," << len1 << "," << len2 << ","
                << std::fixed << std::setprecision(3)
                << avg << "," << min << "," << max << "," << std_dev << std::endl;
        }
        std::cout << "div testing complete, length1: " << len1 << std::endl;
    }

        // 测试num2binary
        for (int len : lengths)
        {
            std::vector<double> times(repetitions, 0);
            for (int i = 0; i < repetitions; ++i)
            {
                times[i] = test_num2binary(len);
            }

            // 计算统计值
            double sum = 0, min = times[0], max = times[0];
            for (double t : times)
            {
                sum += t;
                if (t < min)
                    min = t;
                if (t > max)
                    max = t;
            }
            double avg = sum / repetitions;

            // 计算标准差
            double variance = 0;
            for (double t : times)
            {
                variance += (t - avg) * (t - avg);
            }
            double std_dev = std::sqrt(variance / repetitions);

            // 写入CSV，第二个长度留空
            out << "num2binary," << len << ",,"
                << std::fixed << std::setprecision(3)
                << avg << "," << min << "," << max << "," << std_dev << std::endl;
        }
        std::cout << "num2binary testing complete" << std::endl;

        // 测试binary2num
        for (int len : lengths)
        {
            std::vector<double> times(repetitions, 0);
            for (int i = 0; i < repetitions; ++i)
            {
                times[i] = test_binary2num(len);
            }

            // 计算统计值
            double sum = 0, min = times[0], max = times[0];
            for (double t : times)
            {
                sum += t;
                if (t < min)
                    min = t;
                if (t > max)
                    max = t;
            }
            double avg = sum / repetitions;

            // 计算标准差
            double variance = 0;
            for (double t : times)
            {
                variance += (t - avg) * (t - avg);
            }
            double std_dev = std::sqrt(variance / repetitions);

            // 写入CSV，第二个长度留空
            out << "binary2num," << len << ",,"
                << std::fixed << std::setprecision(3)
                << avg << "," << min << "," << max << "," << std_dev << std::endl;
        }
        std::cout << "binary2num testing complete" << std::endl;

        out.close();
        std::cout << "All tests complete, results saved to: " << filename << std::endl;
    }
