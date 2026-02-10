#include <fstream>  
#include "include/benchmark.hpp"

int main() {
    std::cout << bench_hash(1000000, 1) << std::endl;
    std::cout << bench_hash(1000000, 0) << std::endl;
    return 0;
}