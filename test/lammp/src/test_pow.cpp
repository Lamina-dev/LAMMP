#include "../include/test_short.hpp"
#include <chrono>

void test_pow() {
    const mp_size_t an = 1;
    mp_limb_t a[an];
    srand(123);
    a[0] = 1;
    for (size_t i = 1; i < an; ++i) {
        a[i] = (uint64_t)rand() * rand();
    }
    mp_limb_t exp = 1;
    mp_size_t bn = lmmp_pow_size_(a, an, exp);
    mp_ptr b = (mp_ptr)lmmp_alloc(bn * sizeof(mp_limb_t));
    mp_ptr c = (mp_ptr)lmmp_alloc(bn * sizeof(mp_limb_t));
    auto start = std::chrono::high_resolution_clock::now();
    mp_size_t n2 = lmmp_8_pow_1_(c, bn, a[0], exp);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "duration1 = " << duration << " microseconds" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    mp_size_t n1 = lmmp_pow_win2_(b, a, an, exp);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "duration2 = " << duration << " microseconds" << std::endl;
    std::cout << "bn = " << bn << std::endl;
    std::cout << "n1 = " << n1 << std::endl;
    std::cout << "n2 = " << n2 << std::endl;
    std::cout << "b = ";
    for (size_t i = n1 - 1; i != (size_t)-1; --i) {
        std::cout << std::hex << b[i];
    }
    std::cout << std::endl;
    std::cout << "c = ";
    for (size_t i = n2 - 1; i != (size_t)-1; --i) {
        std::cout << std::hex << c[i];
    }
}