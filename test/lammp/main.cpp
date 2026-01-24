#include <iostream>
#include "include/test_long.hpp"
#include "include/test_short.hpp"

int main() {
    test_short::test_abs_mul64_base();
    test_short::test_div();
    test_short::test_pow();
    return 0;
}