#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>

#include "../../include/lammp/lampz.h"

int main() {
    lampz_t z;
    __lampz_init(z);

    // set a value
    lampz_set_ui(z, 1ull);

    // left shift （内部由二进制实现，此操作很快）
    lampz_lshift(z, 1ull << 24);
    std::cout << "shift finished\n";

    // print the value
    std::string str(lampz_to_str_len(z, 10), '\0');

    // 转换成字符串将进行进制转化，时间复杂度近似为 O(n log n)
    auto start = std::chrono::high_resolution_clock::now();
    size_t len = lampz_to_str(str.data(), str.size(), z, 10);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "to decimal string time taken: "
              << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " microseconds"
              << std::endl;

    std::cout << "Result: (high 32 numbers) : ";
    for (size_t i = len - 1; i >= len - 32; i--) {
        std::cout << str[i];
    }
    std::cout << "....\n";

    // free memory
    lampz_free(z);
    return 0;
}