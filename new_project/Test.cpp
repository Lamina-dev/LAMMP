#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "Test.hpp"

bool is_writable(const std::filesystem::path& p) {
    std::filesystem::perms perm = std::filesystem::status(p).permissions();
    return (perm & std::filesystem::perms::owner_write) != std::filesystem::perms::none;
}

// 生成固定长度的随机整数向量
lammp::_internal_buffer<0> generateRandomIntVector(size_t length, uint64_t min_val, uint64_t max_val) {
    lammp::_internal_buffer<0> vec(length);

    unsigned seed = 120;
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<uint64_t> distribution(min_val, max_val);

    for (size_t i = 0; i < length; ++i) {
        vec.set(i, distribution(generator));
    }

    return vec;
}

std::vector<uint64_t> generateRandomIntVector_(size_t length, uint64_t min_val, uint64_t max_val) {
    std::vector<uint64_t> vec(length);

    unsigned seed = 120;
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<uint64_t> distribution(min_val, max_val);

    for (size_t i = 0; i < length; ++i) {
        vec[i] = distribution(generator);
    }

    return vec;
}

double test_add(int len1, int len2) {
    using namespace lammp;
    using namespace lammp::Arithmetic;

    _internal_buffer<0> vec1 = generateRandomIntVector(get_add_len(len1, len2));
    _internal_buffer<0> vec2 = generateRandomIntVector(len2);

    auto start = std::chrono::high_resolution_clock::now();
    abs_add_binary(vec1.data(), len1, vec2.data(), len2, vec1.data());
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count();
}

double test_sub(int len1, int len2) {
    using namespace lammp;
    using namespace lammp::Arithmetic;

    _internal_buffer<0> vec1 = generateRandomIntVector(get_sub_len(len1, len2));
    _internal_buffer<0> vec2 = generateRandomIntVector(len2);

    auto start = std::chrono::high_resolution_clock::now();
    abs_sub_binary(vec1.data(), len1, vec2.data(), len2, vec1.data());
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count();
}

double test_mul(int len1, int len2) {
    using namespace lammp;
    using namespace lammp::Arithmetic;

    _internal_buffer<0> vec1 = generateRandomIntVector(len1);
    _internal_buffer<0> vec2 = generateRandomIntVector(len2);
    _internal_buffer<0> res(get_mul_len(len1, len2));
    auto start = std::chrono::high_resolution_clock::now();
    abs_mul64(vec1.data(), len1, vec2.data(), len2, res.data());
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count();
}

double test_div(int len1, int len2) {
    using namespace lammp;
    using namespace lammp::Arithmetic;

    _internal_buffer<0> vec1 = generateRandomIntVector(len1);
    _internal_buffer<0> vec2 = generateRandomIntVector(len2);
    _internal_buffer<0> res(get_div_len(len1, len2));
    // 确保除数不为零
    for (auto& val : vec2) {
        if (val == 0)
            val = 1;
    }

    auto start = std::chrono::high_resolution_clock::now();
    // abs_div64(vec1.data(), len1, vec2.data(), len2, res.data());
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count();
}

double test_num2binary(int len) {
    using namespace lammp;
    using namespace lammp::Arithmetic::Numeral;

    const double base_d = BaseTable::table2[8];
    const uint64_t base_num = BaseTable::table1[8][0];
    _internal_buffer<0> vec = generateRandomIntVector(len, 0, base_num);
    _internal_buffer<0> res(get_buffer_size(len, 1 / base_d) + 1);
    auto start = std::chrono::high_resolution_clock::now();
    base2num(vec.data(), len, 10, res.data());
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count();
}

double test_binary2num(int len) {
    using namespace lammp;
    using namespace lammp::Arithmetic::Numeral;
    _internal_buffer<0> vec = generateRandomIntVector(len);
    const double base_d = BaseTable::table2[8];
    const uint64_t base_num = BaseTable::table1[8][0];
    _internal_buffer<0> res(get_buffer_size(len, base_d));
    auto start = std::chrono::high_resolution_clock::now();
    num2base(vec.data(), len, 10, res.data());
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count();
}

// 运行测试并将结果写入CSV文件
void run_tests_and_save(const std::string& filename, const std::vector<int>& lengths, int repetitions) {
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "could't open file: " << filename << std::endl;
        // 检查目录是否存在
        std::filesystem::path dir = std::filesystem::path(filename).parent_path();
        if (!std::filesystem::exists(dir)) {
            std::cerr << "no exist: " << dir << std::endl;
        } else if (!std::filesystem::is_directory(dir)) {
            std::cerr << "not a directory: " << dir << std::endl;
        } else if (!is_writable(dir)) {
            std::cerr << "no write permission: " << dir << std::endl;
        }
        return;
    }

    // 写入CSV头部
    out << "opreate,len1(64bits),len2(64bits),mean time(us),min time(us),max time(us),stddev(us)" << std::endl;

    // 测试加法
    for (int len1 : lengths) {
        for (int len2 : lengths) {
            std::vector<double> times(repetitions, 0);

            for (int i = 0; i < repetitions; ++i) {
                times[i] = test_add(len1, len2);
            }

            // 计算统计值
            double sum = 0, min = times[0], max = times[0];
            for (double t : times) {
                sum += t;
                if (t < min)
                    min = t;
                if (t > max)
                    max = t;
            }
            double avg = sum / repetitions;

            // 计算标准差
            double variance = 0;
            for (double t : times) {
                variance += (t - avg) * (t - avg);
            }
            double std_dev = std::sqrt(variance / repetitions);
            // 写入CSV
            out << "add," << len1 << "," << len2 << "," << std::fixed << avg << "," << min << "," << max << ","
                << std_dev << std::endl;
        }
        std::cout << "add testing complete, length1: " << len1 << std::endl;
    }

    // 测试减法
    for (int len1 : lengths) {
        for (int len2 : lengths) {
            std::vector<double> times(repetitions, 0);
            for (int i = 0; i < repetitions; ++i) {
                times[i] = test_sub(len1, len2);
            }

            // 计算统计值
            double sum = 0, min = times[0], max = times[0];
            for (double t : times) {
                sum += t;
                if (t < min)
                    min = t;
                if (t > max)
                    max = t;
            }
            double avg = sum / repetitions;

            // 计算标准差
            double variance = 0;
            for (double t : times) {
                variance += (t - avg) * (t - avg);
            }
            double std_dev = std::sqrt(variance / repetitions);

            // 写入CSV
            out << "sub," << len1 << "," << len2 << "," << std::fixed << std::setprecision(3) << avg << "," << min
                << "," << max << "," << std_dev << std::endl;
        }
        std::cout << "sub testing complete, length1: " << len1 << std::endl;
    }

    // 测试乘法
    for (int len1 : lengths) {
        for (int len2 : lengths) {
            std::vector<double> times(repetitions, 0);
            for (int i = 0; i < repetitions; ++i) {
                times[i] = test_mul(len1, len2);
            }

            // 计算统计值
            double sum = 0, min = times[0], max = times[0];
            for (double t : times) {
                sum += t;
                if (t < min)
                    min = t;
                if (t > max)
                    max = t;
            }
            double avg = sum / repetitions;

            // 计算标准差
            double variance = 0;
            for (double t : times) {
                variance += (t - avg) * (t - avg);
            }
            double std_dev = std::sqrt(variance / repetitions);

            // 写入CSV
            out << "mul," << len1 << "," << len2 << "," << std::fixed << std::setprecision(3) << avg << "," << min
                << "," << max << "," << std_dev << std::endl;
        }
        std::cout << "mul testing complete, length1: " << len1 << std::endl;
    }

    // 测试除法

    for (int len1 : lengths) {  // 只选择小于 len1 的 len2 进行测试
        for (int len2 : lengths) {
            if (len1 <= len2)  // 跳过 len1 不大于 len2 的情况
                continue;
            std::vector<double> times(repetitions, 0);
            for (int i = 0; i < repetitions; ++i) {
                times[i] = test_div(len1, len2);
            }
            // 计算统计值
            double sum = 0, min = times[0], max = times[0];
            for (double t : times) {
                sum += t;
                if (t < min)
                    min = t;
                if (t > max)
                    max = t;
            }
            double avg = sum / repetitions;
            // 计算标准差
            double variance = 0;
            for (double t : times) {
                variance += (t - avg) * (t - avg);
            }
            double std_dev = std::sqrt(variance / repetitions);
            // 写入 CSV
            out << "div," << len1 << "," << len2 << "," << std::fixed << std::setprecision(3) << avg << "," << min
                << "," << max << "," << std_dev << std::endl;
        }
        std::cout << "div testing complete, length1: " << len1 << std::endl;
    }

    // 测试num2binary
    for (int len : lengths) {
        std::vector<double> times(repetitions, 0);
        for (int i = 0; i < repetitions; ++i) {
            times[i] = test_num2binary(len);
        }

        // 计算统计值
        double sum = 0, min = times[0], max = times[0];
        for (double t : times) {
            sum += t;
            if (t < min)
                min = t;
            if (t > max)
                max = t;
        }
        double avg = sum / repetitions;

        // 计算标准差
        double variance = 0;
        for (double t : times) {
            variance += (t - avg) * (t - avg);
        }
        double std_dev = std::sqrt(variance / repetitions);

        // 写入CSV，第二个长度留空
        out << "num2binary," << len << ",," << std::fixed << std::setprecision(3) << avg << "," << min << "," << max
            << "," << std_dev << std::endl;
    }
    std::cout << "num2binary testing complete" << std::endl;

    // 测试binary2num
    for (int len : lengths) {
        std::vector<double> times(repetitions, 0);
        for (int i = 0; i < repetitions; ++i) {
            times[i] = test_binary2num(len);
        }

        // 计算统计值
        double sum = 0, min = times[0], max = times[0];
        for (double t : times) {
            sum += t;
            if (t < min)
                min = t;
            if (t > max)
                max = t;
        }
        double avg = sum / repetitions;

        // 计算标准差
        double variance = 0;
        for (double t : times) {
            variance += (t - avg) * (t - avg);
        }
        double std_dev = std::sqrt(variance / repetitions);

        // 写入CSV，第二个长度留空
        out << "binary2num," << len << ",," << std::fixed << std::setprecision(3) << avg << "," << min << "," << max
            << "," << std_dev << std::endl;
    }
    std::cout << "binary2num testing complete" << std::endl;

    out.close();
    std::cout << "All tests complete, results saved to: " << filename << std::endl;
}

void test_mul_balance() {
    using namespace lammp;
    using namespace lammp::Arithmetic;
    using namespace lammp::Arithmetic::Numeral;
    size_t len2 = 2000;
    size_t len1 = len2 * 2000;
    _internal_buffer<0> vec1 = generateRandomIntVector(len1);
    _internal_buffer<0> vec2 = generateRandomIntVector(len2);

    size_t res_len = get_mul_len(len1, len2);
    _internal_buffer<0> res1(res_len), res2(res_len), res3(res_len);

    auto start = std::chrono::high_resolution_clock::now();
    abs_mul64_ntt(vec1.data(), len1, vec2.data(), len2, res1.data());
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "time1: " << duration.count() << " us" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    abs_mul64_ntt_unbalanced(vec1.data(), len1, vec2.data(), len2, sqrt(len1 / len2), res2.data());
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "time2: " << duration.count() << " us" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    abs_mul64(vec1.data(), len1, vec2.data(), len2, res3.data());
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "time3: " << duration.count() << " us" << std::endl;

    // for (size_t i = 0; i < 10; ++i) {
    //     std::cout << "res1: " << res1[i] << std::endl;
    //     std::cout << "res2: " << res2[i] << std::endl;
    //     std::cout << "res3: " << res3[i] << std::endl;
    //     // if (res1[i] != res2[i] || res1[i] != res3[i]) {
    //     //     std::cout << "error: " << i << std::endl;
    //     //     std::cout << "res1: " << res1[i] << std::endl;
    //     //     std::cout << "res2: " << res2[i] << std::endl;
    //     //     std::cout << "res3: " << res3[i] << std::endl;
    //     //     break;
    //     // }
    //     // if (false) {
    //     //     std::cout << "checking: " << i << std::endl;
    //     //     std::cout << "vec1: " << vec1[i % len1] << std::endl;
    //     //     std::cout << "vec2: " << vec2[i % len2] << std::endl;
    //     //     std::cout << "res1: " << res1[i] << std::endl;
    //     //     std::cout << "res2: " << res2[i] << std::endl;
    //     //     std::cout << "res3: " << res3[i] << std::endl;
    //     // }
    // }
    return;
}

void test_div_128() {
    using namespace lammp;
    using namespace lammp::Transform;
    using namespace lammp::Transform::number_theory;
    size_t len = 1000000;
    auto vec1 = generateRandomIntVector_(len, 0, 1000000);
    auto vec2 = generateRandomIntVector_(len, 0, UINT64_MAX);
    auto vec3 = generateRandomIntVector_(len, UINT32_MAX, UINT64_MAX);
    std::vector<uint64_t> res1(len), res2(len), res3(len), res4(len);

    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < len; i++) {
        uint64_t tmp = vec2[i];
        res2[i] = div128by64to64_base(vec1[i], tmp, vec3[i]);
        res1[i] = tmp;
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "div 128 by 64 to 64 base time: " << duration.count() << " us" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < len; i++) {
        uint64_t tmp = vec2[i];
        res4[i] = div128by64to64(vec1[i], tmp, vec3[i]);
        res3[i] = tmp;
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "div 128 by 64 to 64 fast time: " << duration.count() << " us" << std::endl;

    for (size_t i = 0; i < len; i++) {
        if (res1[i] != res3[i] || res2[i] != res4[i]) {
            std::cout << "error: " << i << std::endl;
            std::cout << "res1: " << res1[i] << std::endl;
            std::cout << "res2: " << res2[i] << std::endl;
            std::cout << "res3: " << res3[i] << std::endl;
            std::cout << "res4: " << res4[i] << std::endl;
            break;
        }
    }
    return;
}

inline void mul64x64to128_buildin(uint64_t a, uint64_t b, uint64_t& low, uint64_t& high) {
#if defined(UMUL128)
    // #pragma message("Using _umul128 to compute 64bit x 64bit to 128bit")
    unsigned long long lo, hi;
    lo = _umul128(a, b, &hi);
    low = lo, high = hi;
#else
#if defined(UINT128T)  // No _umul128
    // #pragma message("Using __uint128_t to compute 64bit x 64bit to 128bit")
    __uint128_t x(a);
    x *= b;
    low = uint64_t(x), high = uint64_t(x >> 64);
#else                  // No __uint128_t and no _umul128
    // #pragma message("Using basic function to compute 64bit x 64bit to 128bit")
    mul64x64to128_base(a, b, low, high);
#endif                 // UINT128T
#endif                 // UMUL128
}

void test_mul_128() {
    using namespace lammp;
    using namespace lammp::Transform;
    using namespace lammp::Transform::number_theory;
    size_t len = 1250000;
    auto vec1 = generateRandomIntVector_(len);
    auto vec2 = generateRandomIntVector_(len);
    std::vector<uint64_t> res1(len), res2(len), res3(len), res4(len), res5(len), res6(len);

    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < len; i++) {
        uint64_t tmp1, tmp2;
        mul64x64to128_base(vec1[i], vec2[i], res1[i], res2[i]);
        mul64x64to128_base(res1[i], vec2[i], tmp1, res2[i]);
        mul64x64to128_base(res2[i], tmp1, res1[i], tmp2);
        mul64x64to128_base(res1[i], vec2[i], tmp1, res2[i]);
        mul64x64to128_base(res1[i], vec2[i], tmp1, res2[i]);
        mul64x64to128_base(res2[i], tmp1, res1[i], tmp2);
        mul64x64to128_base(res1[i], vec2[i], tmp1, res2[i]);
        mul64x64to128_base(tmp1, tmp2, res1[i], res2[i]);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "mul 64x64 to 128 base time:    " << duration.count() << " us" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < len; i++) {
        uint64_t tmp1, tmp2;
        mul64x64to128(vec1[i], vec2[i], res3[i], res4[i]);
        mul64x64to128(res3[i], vec2[i], tmp1, res4[i]);
        mul64x64to128(res4[i], tmp1, res3[i], tmp2);
        mul64x64to128(res3[i], vec2[i], tmp1, res4[i]);
        mul64x64to128(res3[i], vec2[i], tmp1, res4[i]);
        mul64x64to128(res4[i], tmp1, res3[i], tmp2);
        mul64x64to128(res3[i], vec2[i], tmp1, res4[i]);
        mul64x64to128(tmp1, tmp2, res3[i], res4[i]);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "mul 64x64 to 128 fast time:    " << duration.count() << " us" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < len; i++) {
        uint64_t tmp1, tmp2;
        mul64x64to128_buildin(vec1[i], vec2[i], res5[i], res6[i]);
        mul64x64to128_buildin(res5[i], vec2[i], tmp1, res6[i]);
        mul64x64to128_buildin(res6[i], tmp1, res5[i], tmp2);
        mul64x64to128_buildin(res5[i], vec2[i], tmp1, res6[i]);
        mul64x64to128_buildin(res5[i], vec2[i], tmp1, res6[i]);
        mul64x64to128_buildin(res6[i], tmp1, res5[i], tmp2);
        mul64x64to128_buildin(res5[i], vec2[i], tmp1, res6[i]);
        mul64x64to128_buildin(tmp1, tmp2, res5[i], res6[i]);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "mul 64x64 to 128 buildin time: " << duration.count() << " us" << std::endl;

    for (size_t i = 0; i < len; i++) {
        if (res1[i] != res3[i] || res2[i] != res4[i] || res1[i] != res5[i] || res2[i] != res6[i]) {
            std::cout << "error: " << i << std::endl;
            std::cout << "res1: " << res1[i] << std::endl;
            std::cout << "res2: " << res2[i] << std::endl;
            std::cout << "res3: " << res3[i] << std::endl;
            std::cout << "res4: " << res4[i] << std::endl;
            std::cout << "res5: " << res5[i] << std::endl;
            std::cout << "res6: " << res6[i] << std::endl;
            break;
        }
    }
    return;
}

void test_mul_192() {
    using namespace lammp;
    using namespace lammp::Transform;
    using namespace lammp::Transform::number_theory;
    size_t len = 1000000;
    auto vec1 = generateRandomIntVector_(len);
    auto vec2 = generateRandomIntVector_(len);
    auto vec3 = generateRandomIntVector_(len);
    std::vector<uint64_t> res1(len), res2(len), res3(len), res4(len), res5(len), res6(len);

    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < len; i++) {
        _uint128 tmp(vec1[i], vec2[i]);
        _uint192 tmp2 = _uint192::mul128x64(tmp, vec3[i]);
        res1[i] = (uint64_t)tmp2;
        tmp2 = tmp2.rShift64();
        res2[i] = (uint64_t)tmp2;
        tmp2 = tmp2.rShift64();
        res3[i] = (uint64_t)tmp2;
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "128x64 base time: " << duration.count() << " us" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < len; i++) {
        _uint128 tmp(vec1[i], vec2[i]);
        _uint192 tmp2 = _uint192::mul128x64_fast(tmp, vec3[i]);
        res4[i] = (uint64_t)tmp2;
        tmp2 = tmp2.rShift64();
        res5[i] = (uint64_t)tmp2;
        tmp2 = tmp2.rShift64();
        res6[i] = (uint64_t)tmp2;
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "128x64 fast time: " << duration.count() << " us" << std::endl;

    for (size_t i = 0; i < len; i++) {
        if (res1[i] != res4[i] || res2[i] != res5[i] || res3[i] != res6[i]) {
            std::cout << "error: " << i << std::endl;
            std::cout << "res1: " << res1[i] << std::endl;
            std::cout << "res2: " << res2[i] << std::endl;
            std::cout << "res3: " << res3[i] << std::endl;
            std::cout << "res4: " << res4[i] << std::endl;
            std::cout << "res5: " << res5[i] << std::endl;
            std::cout << "res6: " << res6[i] << std::endl;
            break;
        }
    }

    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < len; i++) {
        _uint192 tmp2 = _uint192::mul64x64x64(vec1[i], vec2[i], vec3[i]);
        res1[i] = (uint64_t)tmp2;
        tmp2 = tmp2.rShift64();
        res2[i] = (uint64_t)tmp2;
        tmp2 = tmp2.rShift64();
        res3[i] = (uint64_t)tmp2;
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "64x64x64 base time: " << duration.count() << " us" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < len; i++) {
        _uint192 tmp2 = _uint192::mul64x64x64_fast(vec1[i], vec2[i], vec3[i]);
        res4[i] = (uint64_t)tmp2;
        tmp2 = tmp2.rShift64();
        res5[i] = (uint64_t)tmp2;
        tmp2 = tmp2.rShift64();
        res6[i] = (uint64_t)tmp2;
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "64x64x64 fast time: " << duration.count() << " us" << std::endl;

    for (size_t i = 0; i < len; i++) {
        if (res1[i] != res4[i] || res2[i] != res5[i] || res3[i] != res6[i]) {
            std::cout << "error: " << i << std::endl;
            std::cout << "res1: " << res1[i] << std::endl;
            std::cout << "res2: " << res2[i] << std::endl;
            std::cout << "res3: " << res3[i] << std::endl;
            std::cout << "res4: " << res4[i] << std::endl;
            std::cout << "res5: " << res5[i] << std::endl;
            std::cout << "res6: " << res6[i] << std::endl;
            break;
        }
    }

    return;
}

void test_mod_div() {
    using namespace lammp;
    using namespace lammp::Arithmetic;
    lamp_ui mod_num = 0x005978fe8134ff32;
    DivSupporter<lamp_ui, _uint128> div(mod_num);
    size_t num = 1000000;
    auto vec1 = generateRandomIntVector_(num, 0, mod_num);
    auto vec2 = generateRandomIntVector_(num, 0, mod_num);
    std::vector<lamp_ui> res1(num), res2(num), res3(num), res4(num);

    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < num; i++) {
        div.prodDivMod(vec1[i], vec2[i], res1[i], res2[i]);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "div supporter1 time: " << duration.count() << " us" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < num; i++) {
        uint64_t tmp_high, tmp_low;
        mul64x64to128(vec1[i], vec2[i], tmp_low, tmp_high);
        res4[i] = div128by64to64(tmp_high, tmp_low, mod_num);
        res3[i] = tmp_low;
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "div128by64to64 time: " << duration.count() << " us" << std::endl;

    for (size_t i = 0; i < num; i++) {
        if (res1[i] != res4[i] || res2[i] != res3[i]) {
            std::cout << "error: " << i << std::endl;
            std::cout << "res1: " << res1[i] << std::endl;
            std::cout << "res2: " << res2[i] << std::endl;
            std::cout << "res3: " << res3[i] << std::endl;
            std::cout << "res4: " << res4[i] << std::endl;
            break;
        }
    }
}

void test_barrett_2powN_div_num() {
    using namespace lammp;
    using namespace lammp::Arithmetic;
    size_t N = 11292;
    size_t len = 1112;
    size_t res_len = get_div_len(N + 1, len);
    auto vec1 = generateRandomIntVector_(len);

    std::vector<lamp_ui> res1(res_len, 0), res2(N + 2, 0);
    vec1[len - 1] = 0xe2fffffffffffff2;

    auto start = std::chrono::high_resolution_clock::now();
    res_len = barrett_2powN_div_num(N, vec1.data(), len, res1.data());
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "barrett_2powN_div_num time: " << duration.count() << " us" << std::endl;

    abs_mul64(res1.data(), res_len, vec1.data(), len, res2.data());
    size_t res2_len = rlz(res2.data(), get_mul_len(res_len, len));
    for (size_t i = res2_len - 5; i < res2_len; i++) {
        std::cout << i << ": " << res2[i] << std::endl;
    }
    std::cout << std::endl;
    return;
}
