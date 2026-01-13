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
    int a = 1<<20;
    std::cout << bench_mul(a, a) << std::endl;
    return 0;
}
