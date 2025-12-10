# Lammp
Lamina的任意精度计算库。

提供类似GMP的任意精度计算能力，目前支持有符号整数。Lammp的目标是提供一个快速、可靠、可扩展的任意精度计算库，性能基本与GMP相当。

基本类型：

- 整数：有符号整数 ``lampz_t``为结构体指针，该结构体指针内部存储大整数数组。数组默认采用64位整数。尽管lammp生成可以支持任意精度的整数，但在理论中，所能够支持的最大的整数不超过$$2^((2^64-1)*64)$$。当然要存储这个数字，已经是目前的64位计算机内存所能达到的上限了。

计算函数可以见于``lampz.h``中，该文件提供 C 接口。

编译：

目前Lammp仅支持Windows平台，采用mingw编译器，需要CMake编译。

一个简单的示例：

```c++
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>
#include <algorithm>
#include "include/lammp/lampz.h"


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
```
