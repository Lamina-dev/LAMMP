#include <fstream>  
#include "include/benchmark.hpp"
#include <iostream>
#include <chrono>


int main() {
    lmmp_global_init();
    std::cout << bench_factorial(10000000) << std::endl;
    lmmp_global_deinit();
    return 0;
}