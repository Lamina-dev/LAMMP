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
    test_mod_div();
    return 0;
}