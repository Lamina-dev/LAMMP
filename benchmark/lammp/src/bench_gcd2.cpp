#include "../include/benchmark.hpp"
#include <vector>

void bench_gcd2() {
    int len = 1000000;
    std::vector<mp_limb_t[2]> a(len), b(len);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<mp_limb_t> dis(1, LIMB_MAX);
    for (int i = 0; i < len; i++) {
        a[i][0] = dis(gen);
        a[i][1] = dis(gen);
        b[i][0] = dis(gen);
        b[i][1] = dis(gen);
    }
    std::vector<mp_limb_t[2]> r1(len), r2(len);

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < len; i++) {
        lmmp_gcd_22_(r1.data()[i], a.data()[i], b.data()[i]);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "gcd_22: " << duration << " microseconds" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < len; i++) {
        lmmp_gcd_lehmer_(r2.data()[i], a.data()[i], 2, b.data()[i], 2);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "gcd_22_lehmer: " << duration << " microseconds" << std::endl;

    for (int i = 0; i < len; i++) {
        if (r1[i][0] != r2[i][0] || r1[i][1] != r2[i][1]) {
            std::cout << "i: " << i << std::endl;
            std::cout << "a: " << a[i][0] << " " << a[i][1] << std::endl;
            std::cout << "b: " << b[i][0] << " " << b[i][1] << std::endl;
            std::cout << "r1: " << r1[i][0] << " " << r1[i][1] << std::endl;
            std::cout << "r2: " << r2[i][0] << " " << r2[i][1] << std::endl;
            std::cout << "Error: gcd_22 and gcd_22_lehmer are not equal" << std::endl;
            return;
        }
    }
    std::cout << "Test passed" << std::endl;
    return;
}