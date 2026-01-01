#include "../include/benchmark.hpp"

void bench_div_128() {
    using namespace lammp;
    size_t len = 1000000;
    auto vec1 = generateRandomIntVector_(len, 0, UINT32_MAX - 1);
    auto vec2 = generateRandomIntVector_(len, 0, UINT64_MAX);
    auto vec3 = generateRandomIntVector_(len, UINT32_MAX, UINT64_MAX);
    std::vector<uint64_t> res1(len), res2(len), res3(len), res4(len);

    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < len; i++) {
        uint64_t tmp = vec2[i];
        res2[i] = div128by64to64_base(vec1[i], tmp, vec3[i]);
        res1[i] = tmp;
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "div 128 by 64 to 64 base time: " << duration.count() << " us" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < len; i++) {
        uint64_t tmp = vec2[i];
        res4[i] = div128by64to64(vec1[i], tmp, vec3[i]);
        res3[i] = tmp;
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "div 128 by 64 to 64 fast time: " << duration.count() << " us" << std::endl;

    for (size_t i = 0; i < len; i++) {
        if (res1[i] != res3[i] || res2[i] != res4[i]) {
            std::cout << "error: " << i << std::endl;
            std::cout << "res1: " << res1[i] << std::endl;
            std::cout << "res2: " << res2[i] << std::endl;
            std::cout << "res3: " << res3[i] << std::endl;
            std::cout << "res4: " << res4[i] << std::endl;
            break;
        }
    }
    return;
}