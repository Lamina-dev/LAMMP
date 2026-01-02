#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <iostream>
#include <string>

#include "../../include/lammp/lampz.h"


int main() {
    lampz_t z, x;
    __lampz_init(z);
    __lampz_init(x);

    // set a value
    lampz_set_ui(x, 1234567890);

    std::string str = "1237817927876726574923467362786482368238090123768293486732874";
    std::reverse(str.begin(), str.end());
    lampz_set_str(z, str.c_str(), str.size(), 10);


    lampz_div_x(z, x);

    std::string result(lampz_to_str_len(z, 10), '\0');
    size_t result_len = lampz_to_str(result.data(), result.size(), z, 10);

    result.resize(result_len);
    std::reverse(result.begin(), result.end());

    std::cout << "Result: " << result << std::endl;

    lampz_free(x);
    lampz_free(z);
    return 0;
}
