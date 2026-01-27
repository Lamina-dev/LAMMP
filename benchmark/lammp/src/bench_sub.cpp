#include "../include/benchmark.hpp"

long long bench_sub(int len1, int len2) {
    if (len1 < len2) {
        std::swap(len1, len2);
    } else if (len1 == len2) {
        len1++;
    }
    mp_ptr a = (mp_ptr)lmmp_alloc(len1 * sizeof(mp_limb_t));
    mp_ptr b = (mp_ptr)lmmp_alloc(len2 * sizeof(mp_limb_t));
    mp_ptr c = (mp_ptr)lmmp_alloc(len1 * sizeof(mp_limb_t));
    auto start = std::chrono::high_resolution_clock::now();
    lmmp_sub_(c, a, len1, b, len2);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    lmmp_free(a);
    lmmp_free(b);
    lmmp_free(c);
    return duration.count();
}
