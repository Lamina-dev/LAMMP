#include "../include/benchmark.hpp"

long long bench_add(int len1, int len2) {
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