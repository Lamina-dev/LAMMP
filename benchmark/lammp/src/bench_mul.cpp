#include "../include/benchmark.hpp"

long long bench_mul(int len1, int len2) {
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

long long bench_sqr(int len1) {
    using namespace lammp;
    using namespace lammp::Arithmetic;

    _internal_buffer<0> vec1 = generateRandomIntVector(len1);
    _internal_buffer<0> res(get_mul_len(len1, len1));
    auto start = std::chrono::high_resolution_clock::now();
    abs_mul64(vec1.data(), len1, vec1.data(), len1, res.data());
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count();
}