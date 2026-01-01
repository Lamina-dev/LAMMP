#include "../include/benchmark.hpp"

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