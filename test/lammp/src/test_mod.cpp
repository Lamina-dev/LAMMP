#include "../include/test_short.hpp"
#include <chrono>
#include <vector>

void test_mod() {
    mp_size_t n = 1000000;
    std::vector<mp_limb_t[4]> a(n);
    for (mp_size_t i = 0; i < n; i++) {
        a[i][0] = (i + 1) * rand();
        a[i][1] = (i + 2) * rand();
        a[i][2] = (i + 3) * rand();
        a[i][3] = (i + 4) * rand() + 2;
    }

    mp_limb_t m[2] = {12789, 0xfffffff0000ffff};
    auto start = std::chrono::high_resolution_clock::now();
    for (mp_size_t i = 0; i < n; i++) {
        mp_limb_t t[2] = {m[0], m[1]};
        lmmp_div_2_(a[i], a[i], 4, t);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto d = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "test_mod() time: " << d << " microseconds" << std::endl;
    
}