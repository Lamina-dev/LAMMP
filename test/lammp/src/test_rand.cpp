#include "../include/test_short.hpp"
#include <random>
#include <chrono>

void test_rand() {

    uint64_t seed = 123456789;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<mp_limb_t> dis(0, UINT64_MAX);
    mp_size_t len = 1000000;
    mp_ptr dst = ALLOC_TYPE(len, mp_limb_t);
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < len; i++) {
        dst[i] = dis(gen);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "duration: " << duration << " microseconds" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    lmmp_seed_random_(dst, len, seed, 0);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "duration: " << duration << " microseconds" << std::endl;
    
    start = std::chrono::high_resolution_clock::now();
    lmmp_seed_random_(dst, len, seed, 1);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "duration: " << duration << " microseconds" << std::endl;
    lmmp_free(dst);
}