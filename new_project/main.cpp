#include <iostream>
#include <cstdint>
#include <cassert>
#include <chrono>
#include <random>
#include "src/lammp.hpp"
#include "Test.hpp"
using namespace lammp;
using namespace lammp::Transform::number_theory;

int main()
{
    //test_knuth_div();
    int len = 10000000 / 64;
    std::cout << test_mul(len, len) << std::endl;
    test_barrett_2powN();
    //test_barrett_2powN_div_num();
    return 0;
}