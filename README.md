# Lammp
Lamina的任意精度计算库。

提供类似GMP的任意精度计算能力，目前支持有符号整数。Lammp的目标是提供一个快速、可靠、可扩展的任意精度计算库，性能基本与GMP相当。

## 基本类型

- 整数：有符号整数 ``lampz_t``为 ``__struct_lampz``一维数组的别名，该结构体指针内部存储大整数数组的头尾指针。数组默认采用64位整数。存储整数为其绝对值，负数通过使用负数的数组长度表示。

计算函数可以见于``lampz.h``中，该文件提供 C 接口。

## 编译

目前Lammp仅支持Windows平台，采用mingw编译器，使用CMake编译，目前暂不支持MSVC编译器，后续将支持。

## 快速开始

```c++
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
```
