#include "include/benchmark.hpp"

int main() {
    // for (int i = 0; i < 20; i++) {
    //     int a = 500 * i + 4000;
    //     for (int j = 0; j < 10; j++) {
    //         int b = a * (3 * j + 10);
    //         std::cout << a << "," << b << ",";
    //         std::cout << bench_mul(a, b) << ";";
    //     }
    // }
    // for (int i = 0; i < 300; i++) {
    //     unsigned len1 = 1000 + i * 4000;
    //     std::cout << len1 << "," << bench_factorial(len1) << ";";
    // }
    std::cout << bench_factorial(1000000) << ";";
    return 0;
}
