#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>
#include <algorithm>
#include "include/lammp/lampz.h"
#include "include/test/Test.hpp"
/*
int main() {
    lampz_t z = nullptr;
    std::string str = "0123781792787672657497878419374891734891374913874";
    std::string str_copy(str.size(), '\0');
    std::copy(str.begin(), str.end(), str_copy.begin());
    std::reverse(str_copy.begin(), str_copy.end());
    std::cout << "computer " << str_copy << " + " << str_copy << " = " << std::endl;

    lampz_set_str(z, str.data(), 10);
    if (z == nullptr) {
        std::cout << "Error: memory allocation failed" << std::endl;
        return 1;
    }

    lampz_add_x(z, z);
    std::string str3(lampz_to_str_len(z, 10), '\0');
    lamp_ui str3_len = lampz_to_str(str3.data(), str3.capacity(), z, 10);
    for (size_t i = str3_len; i-- != 0; ) {
        std::cout << str3[i];
    }
    std::cout << std::endl;

    lampz_free(z);
    return 0;
}
*/
int main() {
    test_div_128_support();
    return 0;
}