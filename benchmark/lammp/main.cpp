#include <fstream>  
#include "include/benchmark.hpp"

int main() {
    for (int i = 1; i <= 100; i++) {
        uint t = 10000 * i + 1000;
        std::cout << t << "," << bench_factorial(t) << ";";
    }
    return 0;
}