#include <fstream>  
#include "include/benchmark.hpp"
#include <iostream>
#include <chrono>


int main() {
    lmmp_global_init();
    std::cout << bench_div(5000, 1400) << std::endl;
    lmmp_global_deinit();
    return 0;
}