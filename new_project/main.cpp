#include <iostream>
#include <cstdint>
#include <cassert>
#include <chrono>
#include <random>
#include "src/lammp.hpp"
#include "Test.hpp"
using namespace lammp;
using namespace lammp::Transform::number_theory;

int main() {
    // test_barrett_2powN();
    for (int i = 1; i < 50; i += 2) {
        int N = 1000 * i;
        int len = 500 * i;
        //std::cout << i << " " << (int)test_barrett_pre_div(N, len) << std::endl;
        std::cout << "N = " << N << ", len = " << len << ", time = " << (int)test_barrett_pre_div(N, len) << " us"
                << std::endl;
    }
    
    return 0;
}