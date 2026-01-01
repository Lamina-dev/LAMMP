#include "../include/benchmark.hpp"

long long bench_barrett_pre_div(int N, int len) {
    using namespace lammp;
    using namespace lammp::Arithmetic;

    auto vec1 = generateRandomIntVector_(len);
    size_t res_len = N - len + 1;
    std::vector<lamp_ui> res1(res_len + 1, 0), res2(N + 1, 0);

    auto start = std::chrono::high_resolution_clock::now();
    barrett_2powN(N, vec1.data(), len, res1.data());
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count();
}