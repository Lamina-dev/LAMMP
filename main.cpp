#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>
#include <algorithm>
#include "include/lammp/lampz.h"
#include "include/test/Test.hpp"

int main() {
    lampz_t z;
    __lampz_init(z);
    std::string str = "01237817927876726574923467362786482368238090123768293486732874";
    std::string str_copy(str.size(), '\0');
    std::copy(str.begin(), str.end(), str_copy.begin());
    std::reverse(str_copy.begin(), str_copy.end());
    std::cout << "computer " << str_copy << " + " << str_copy << " = " << std::endl;

    lampz_set_str(z, str.data(), str.size(), 10);
    

    lampz_mul_x(z, z);
    std::string str3(lampz_to_str_len(z, 10), '\0');
    lamp_sz str3_len = lampz_to_str(str3.data(), str3.capacity(), z, 10);
    for (size_t i = str3_len; i-- != 0; ) {
        std::cout << str3[i];
    }
    std::cout << std::endl;

    lampz_free(z);
    return 0;
}
