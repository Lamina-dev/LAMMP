#include "../include/benchmark.hpp"

inline void mul64x64to128_buildin(uint64_t a, uint64_t b, uint64_t& low, uint64_t& high) {
#if defined(UMUL128)
    // #pragma message("Using _umul128 to compute 64bit x 64bit to 128bit")
    unsigned long long lo, hi;
    lo = _umul128(a, b, &hi);
    low = lo, high = hi;
#else
#if defined(UINT128T)  // No _umul128
    // #pragma message("Using __uint128_t to compute 64bit x 64bit to 128bit")
    __uint128_t x(a);
    x *= b;
    low = uint64_t(x), high = uint64_t(x >> 64);
#else                  // No __uint128_t and no _umul128
    // #pragma message("Using basic function to compute 64bit x 64bit to 128bit")
    mul64x64to128_base(a, b, low, high);
#endif                 // UINT128T
#endif                 // UMUL128
}

void bench_mul_128() {
    using namespace lammp;
    size_t len = 1250000;
    auto vec1 = generateRandomIntVector_(len);
    auto vec2 = generateRandomIntVector_(len);
    std::vector<uint64_t> res1(len), res2(len), res3(len), res4(len), res5(len), res6(len);

    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < len; i++) {
        uint64_t tmp1, tmp2;
        mul64x64to128_base(vec1[i], vec2[i], res1[i], res2[i]);
        mul64x64to128_base(res1[i], vec2[i], tmp1, res2[i]);
        mul64x64to128_base(res2[i], tmp1, res1[i], tmp2);
        mul64x64to128_base(res1[i], vec2[i], tmp1, res2[i]);
        mul64x64to128_base(res1[i], vec2[i], tmp1, res2[i]);
        mul64x64to128_base(res2[i], tmp1, res1[i], tmp2);
        mul64x64to128_base(res1[i], vec2[i], tmp1, res2[i]);
        mul64x64to128_base(tmp1, tmp2, res1[i], res2[i]);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "mul 64x64 to 128 base time:    " << duration.count() << " us" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < len; i++) {
        uint64_t tmp1, tmp2;
        mul64x64to128(vec1[i], vec2[i], res3[i], res4[i]);
        mul64x64to128(res3[i], vec2[i], tmp1, res4[i]);
        mul64x64to128(res4[i], tmp1, res3[i], tmp2);
        mul64x64to128(res3[i], vec2[i], tmp1, res4[i]);
        mul64x64to128(res3[i], vec2[i], tmp1, res4[i]);
        mul64x64to128(res4[i], tmp1, res3[i], tmp2);
        mul64x64to128(res3[i], vec2[i], tmp1, res4[i]);
        mul64x64to128(tmp1, tmp2, res3[i], res4[i]);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "mul 64x64 to 128 fast time:    " << duration.count() << " us" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < len; i++) {
        uint64_t tmp1, tmp2;
        mul64x64to128_buildin(vec1[i], vec2[i], res5[i], res6[i]);
        mul64x64to128_buildin(res5[i], vec2[i], tmp1, res6[i]);
        mul64x64to128_buildin(res6[i], tmp1, res5[i], tmp2);
        mul64x64to128_buildin(res5[i], vec2[i], tmp1, res6[i]);
        mul64x64to128_buildin(res5[i], vec2[i], tmp1, res6[i]);
        mul64x64to128_buildin(res6[i], tmp1, res5[i], tmp2);
        mul64x64to128_buildin(res5[i], vec2[i], tmp1, res6[i]);
        mul64x64to128_buildin(tmp1, tmp2, res5[i], res6[i]);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "mul 64x64 to 128 buildin time: " << duration.count() << " us" << std::endl;

    for (size_t i = 0; i < len; i++) {
        if (res1[i] != res3[i] || res2[i] != res4[i] || res1[i] != res5[i] || res2[i] != res6[i]) {
            std::cout << "error: " << i << std::endl;
            std::cout << "res1: " << res1[i] << std::endl;
            std::cout << "res2: " << res2[i] << std::endl;
            std::cout << "res3: " << res3[i] << std::endl;
            std::cout << "res4: " << res4[i] << std::endl;
            std::cout << "res5: " << res5[i] << std::endl;
            std::cout << "res6: " << res6[i] << std::endl;
            break;
        }
    }
    return;
}