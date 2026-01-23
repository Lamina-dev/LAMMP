#include "../include/benchmark.hpp"

long long bench_factorial(unsigned len1) {
    mp_size_t rn = lmmp_factorial_size_(len1);
    mp_ptr dst = ALLOC_TYPE(rn, mp_limb_t);

    auto start = std::chrono::high_resolution_clock::now();
    rn = lmmp_factorial_(dst, rn, len1);
    auto end = std::chrono::high_resolution_clock::now();

    long long elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    lmmp_free(dst);
    return elapsed_us;
}