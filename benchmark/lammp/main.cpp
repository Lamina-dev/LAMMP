#include <fstream>  
#include "include/benchmark.hpp"
#include <iostream>
#include <chrono>


int main() {
    std::cout << bench_factorial(10000000) << std::endl;
    return 0;
}