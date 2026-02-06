#include "../include/test_short.hpp"
#include <chrono>

void test_factorial() {
    size_t n = 0;
    size_t len = lmmp_factorial_size_(n);
    mp_ptr a = ALLOC_TYPE(len, mp_limb_t);
    mp_ptr b = ALLOC_TYPE(len, mp_limb_t);

    a[0] = 1;
    mp_size_t an = 1;
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 1; i <= n; i++) {
        a[an] = lmmp_mul_1_(a, a, an, i);
        ++an;
        an -= a[an - 1] == 0 ? 1 : 0;
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "Time elapsed: (native)" << duration << " microseconds" << std::endl;

    auto start2 = std::chrono::high_resolution_clock::now();
    mp_size_t bn = lmmp_factorial_(b, len, n);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();
    std::cout << "Time elapsed: (queued)" << duration2 << " microseconds" << std::endl;

    std::cout << an << " " << bn << std::endl;
    for (size_t i = 0; i < an; i++) {
        if (a[i] != b[i]) {
            std::cout << "i = " << i << " a = " << a[i] << " b = " << b[i] << std::endl;
            break;
        }
    }
    std::cout << "passed\n";
    lmmp_free(a);
    lmmp_free(b);
}