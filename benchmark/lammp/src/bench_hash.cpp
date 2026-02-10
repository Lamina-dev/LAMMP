#include "../include/benchmark.hpp"
#include "../../../include/lammp/secret.h"

long long bench_hash(int len, int type) {
    type %= 2;
    mp_ptr a = (mp_ptr)lmmp_alloc(len * sizeof(mp_limb_t));
    ger_random_numbers(len, a);
    if (type == 0) {
        auto start = std::chrono::high_resolution_clock::now();
        uint64_t h = lmmp_siphash24_(a, len, (srckey128_t){1012321, 233320});
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        lmmp_free(a);
        return duration.count();
    } else {
        auto start = std::chrono::high_resolution_clock::now();
        uint64_t h = lmmp_xxhash_(a, len, 1012321);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        lmmp_free(a);
        return duration.count();
    }
}