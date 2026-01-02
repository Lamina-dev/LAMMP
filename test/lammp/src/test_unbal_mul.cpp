#include "../include/test_long.hpp"

void test_unbalanced_mul() {
    std::cout << "Testing unbalanced multiplication..." << std::endl;
    size_t len1 = 100000;
    size_t len2 = 1000;
    size_t out_len = len1 + len2;
    std::vector<lammp::Arithmetic::lamp_ui> vec1(len1, 0);
    std::vector<lammp::Arithmetic::lamp_ui> vec2(len2, 0);
    std::vector<lammp::Arithmetic::lamp_ui> out1(out_len, 0);
    std::vector<lammp::Arithmetic::lamp_ui> out2(out_len, 0);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<lammp::Arithmetic::lamp_ui> dis(0, UINT64_MAX);
    for (size_t i = 0; i < len2; i++) {
        vec2[i] = dis(gen);
    } 
    for (size_t i = 0; i < len1; i++) {
        vec1[i] = dis(gen);
    }
    size_t M = sqrt(len1 / len2);
    auto start = std::chrono::high_resolution_clock::now();
    lammp::Arithmetic::abs_mul64_ntt_unbalanced(vec1.data(), len1, vec2.data(), len2, M, out1.data());
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "unbalanced multiplication Done!" << std::endl;
    std::cout << "Time elapsed: " << duration << " microseconds" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    lammp::Arithmetic::abs_mul64_ntt(vec1.data(), len1, vec2.data(), len2, out2.data());
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "ntt multiplication Done!" << std::endl;
    std::cout << "Time elapsed: " << duration << " microseconds" << std::endl;

    for (size_t i = 0; i < out_len; i++) {
        if (out1[i] != out2[i]) {
            std::cout << "Error: out1[" << i << "] = " << out1[i] << " out2[" << i << "] = " << out2[i] << std::endl;
            return;
        }
    }
    std::cout << "Test passed!" << std::endl;
}