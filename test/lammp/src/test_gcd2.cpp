#include <chrono>
#include <vector>
#include "../include/test_short.hpp"


void test_gcd2() {
    mp_limb_t a[2] = {52516872761, 1110};
    mp_limb_t b[2] = {52516872761, 1110};
    mp_limb_t r1[2];
    mp_limb_t r2[2] = {0, 0};
    lmmp_gcd_22_(r1, a, b);
    mp_size_t n = lmmp_gcd_lehmer_(r2, a, 2, b, 2);
    std::cout << r1[0] << " " << r1[1] << std::endl;
    std::cout << r2[0] << " " << r2[1] << std::endl;
    std::cout << n << std::endl;
    return;
}