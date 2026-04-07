#include <fstream>  
#include "include/benchmark.hpp"
#include <iostream>
#include <chrono>


int main() {
    lmmp_stack_init();
    std::cout << bench_factorial(1000000) << std::endl;
    return 0;
}