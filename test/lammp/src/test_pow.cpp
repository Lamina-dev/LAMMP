#include "../include/test_short.hpp"
#include <chrono>

void test_pow() {
    const mp_size_t an = 1;
    mp_limb_t a[an] = {676372192};
    mp_limb_t exp = 82441221;
    mp_size_t bn = lmmp_pow_size_(a, an, exp);
    mp_ptr b = (mp_ptr)lmmp_alloc(bn * sizeof(mp_limb_t));
    mp_ptr c = (mp_ptr)lmmp_alloc(bn * sizeof(mp_limb_t));
    auto start = std::chrono::high_resolution_clock::now();
    mp_size_t n2 = lmmp_pow_1_(c, a[0], exp);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "duration1 = " << duration << " microseconds" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    mp_size_t n1 = lmmp_pow_basecase_(b, a, an, exp);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "duration2 = " << duration << " microseconds" << std::endl;
    std::cout << "bn = " << bn << std::endl;
    std::cout << "n1 = " << n1 << std::endl;
    std::cout << "n2 = " << n2 << std::endl;
    std::cout << "b = ";
    for (size_t i = n1 - 1; i != (size_t)n1 - 10; --i) {
        std::cout << std::hex << b[i];
    }
    std::cout << std::endl;
    std::cout << "c = ";
    for (size_t i = n2 - 1; i != (size_t)n2 - 10; --i) {
        std::cout << std::hex << c[i];
    }
}