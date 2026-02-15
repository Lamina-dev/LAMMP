#include "../include/benchmark.hpp"

long long bench_mul_unbal(int len1, int len2) {
    mp_size_t len = len1 + len2;
    mp_ptr a = (mp_ptr)lmmp_alloc(len1 * sizeof(mp_limb_t));
    mp_ptr b = (mp_ptr)lmmp_alloc(len2 * sizeof(mp_limb_t));
    mp_ptr c = (mp_ptr)lmmp_alloc(len * sizeof(mp_limb_t));

    ger_random_numbers(len1, a);
    ger_random_numbers(len2, b);
    auto start = std::chrono::high_resolution_clock::now();
    lmmp_mul_ntt_unbal_(c, a, len1, b, len2, 0);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    lmmp_free(a);
    lmmp_free(b);
    lmmp_free(c);
    return duration.count();
}