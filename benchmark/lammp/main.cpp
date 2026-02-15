#include <fstream>  
#include "include/benchmark.hpp"

int main() {
    for (int i = 200; i < 300; i += 4) {
        for (int j = 0; j < 80; j++) {
            int len2 = 150 * j + 8000;
            int len1 = len2 * i;
            std::cout << len1 << "," << len2 << "," << bench_mul(len1, len2) << "," << bench_mul_unbal(len1, len2)
                      << ";";
        }
    }
    return 0;
}