#include <stdio.h>
#include <string.h>
#include <iostream>
#include "include/lammp/lampz.h"


int main() {
    lampz_t z = nullptr;

    char str[100] = "ffffffffffffffffa";
    int len = strlen(str);
    std::cout << len << std::endl;
    lampz_set_str(z, str, 16);
    if (z == nullptr) {
        std::cout << "Error: memory allocation failed" << std::endl;
        return 1;
    }
    
    char str2[128] = "\0";
    lamp_ui str2_len = lampz_to_str(str2, 128, z, 10);
    for (size_t i = 0; i < strlen(str2); i++) {
        std::cout << str2[i];
    }
    std::cout << std::endl;
    lampz_free(z);
    return 0;
}