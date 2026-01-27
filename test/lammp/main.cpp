#include <iostream>
#include "include/test_long.hpp"
#include "include/test_short.hpp"
#include <vector>

int main() {
    if (test_pow(12, 65)) {
        printf("test_pow failed\n");
    } else {
        printf("test_pow passed\n");
    }
    return 0;
}