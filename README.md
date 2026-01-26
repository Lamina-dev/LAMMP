# Lammp
Lamina的高精度计算库。

提供类似GMP的任意精度计算能力，同时支持或计划支持包括但不限于如开方、阶乘、组合数、素性检验、质因数分解等复杂的高精度计算。

## 编译

目前Lammp仅支持Windows平台，使用CMake构建，目前可支持MSVC，MinGW编译器，clang编译器暂无详细编译测试结果。

Lammp无标准库外的依赖，动态库LammpCore为纯c和汇编编写，无需其他依赖。

目前仅支持采用MASM汇编器，NASM汇编器正在计划中。

## 架构

目前，非x86-64平台暂不支持，后续考虑将会支持其他64位平台，32位平台暂无计划。

## 文档

目前，Lammp的文档还在编写中，在``doc/lammp/dev``下，已有部分文档描述，文档标准暂未制定，考虑到实用性，可能使用Markdown格式。

欢迎参与编写。

## 目录结构

```
MAIN/                       # 项目根目录
├── LICENSE                 # 许可证文件
├── README.md               # README
├── CMakeLists.txt          # 根目录CMake（全局配置：构建类型、C/C++标准、输出目录等）
├── main.c                  # 项目主程序（编译后生成LammpMain可执行文件）
├── dist/                   # 编译产物根目录（自动生成，存放所有库和可执行文件）
│   └── lammp/              # Lammp项目专属产物目录（隔离其他库）
│       ├── bin/            # 可执行文件输出目录
│       └── lib/            # 动态库输出目录
├── include/                # 头文件目录（对外暴露，所有子模块可引用）
│   └── lammp/              # 项目名嵌套目录
├── src/                    # 核心源代码根目录
│   └── lammp/              # 核心库源代码目录（对应include/lammp）
│       ├── CMakeLists.txt  # 核心动态库专属CMake（编译LammpCore）
│       ├── LammpCore.def   # MSVC专属定义动态库导出文件（MSVC编译时需要）
│       ├── NumTh/          # 数论计算子模块实现文件
│       ├── secret/         # 密码学子模块实现文件
│       ├── mprand/         # 随机数生成子模块实现文件
│       ├── masm/.asm       # 实现文件
│       └── .c              # 实现文件
├── benchmark/              # 基准测试根目录
│   └── lammp/              # Lammp项目基准测试目录
│       ├── CMakeLists.txt  # 基准测试CMake配置
│       ├── include/        # 基准测试私有头文件（仅测试内部使用）
│       ├── src/            # 基准测试源代码目录
│       └── main.cpp        # 基准测试主程序（入口函数main()）
├── example/                # 示例程序根目录
│   └── lammp/              # Lammp项目基准测试目录
│       ├── CMakeLists.txt  # 示例程序CMake配置
│       ├── example1.cpp    # 示例1
│       └── example2.cpp    # 示例2
├── test/                   # 测试程序根目录
│   └── lammp/              # Lammp项目基准测试目录
│       ├── CMakeLists.txt  # 测试程序CMake配置
│       ├── include/        # 测试私有头文件（仅测试内部使用）
│       ├── src/            # 测试源代码目录
│       └── main.cpp        # 测试主程序入口函数main()
└── build/                  # 构建目录（外部构建，仅供参考）
    ├── CMakeCache.txt      # CMake缓存文件（自动生成）
    ├── Makefile            # 编译脚本（Linux/Mac，自动生成）
    └── else
```



## 快速开始

```c++
#include <string.h>
#include <algorithm>
#include <iostream>
#include <string>

#include "../../include/lammp/lampz.h"

int main() {
    lampz_t z;
    __lampz_init(z); /* 初始化为 NAN 也即空 */
    // 请注意，初始化为空参与计算，将导致计算结果也为空，这是明确行为，而不是未定义行为。
    // 因此，请务必在使用其计算前注意是否为空。
    // 例如，使用 lampz_add_x(z, x); 若 x 为空，则结果也为空，z 的内部内存将会被安全释放，即 z 变为 NAN 空。
    // lampz_new(z); /* 申请 0 比特的内存，z 的数值将会设置为零，将会至少分配一个字长的空间 */
    // lampz_new(z, 1024); /* 申请 1024 比特的内存，z 的数值将会设置为零，无论空间有多大 */

    /* 小端序的字符串，0 为最低有效位 1 为第二位，以此类推 */
    /* 输入的字符将不会做任何检查，非法输入将导致未定义行为 */
    /* 支持2-36进制，但不支持负数，即输入和输出的字符串均为绝对值，此为明确行为，而不是未定义行为 */
    /* 注意：字符串长度不一定等于比特长度，请注意区分 */
    /* 大小写均可，但输出的字符串将会全部使用小写字母 */
    /* 对于十进制以上进制，将使用 a - z 作为字符，依次表示 10 - 35 */
    std::string str = "01237817927876726574923467362786482368238090123768293486732874";
    std::string str_copy(str.size(), '\0');
    std::copy(str.begin(), str.end(), str_copy.begin());
    std::reverse(str_copy.begin(), str_copy.end());
    std::cout << "computer " << str_copy << " + " << str_copy << " = " << std::endl;

    /* 字符串转化为 lampz 类型，z 可以为空，自动分配内存 */
    lampz_set_str(z, str.data(), str.size(), 10);

    /* 计算 z *= z */
    lampz_mul_x(z, z);
    // lampz_mul_xy(z, z, z); // 等价

    /* 输出 z 的字符串表示 */
    /* 注意：字符串缓冲区长度由函数lampz_to_str_len()返回 */
    /* 注意：此值请与你实际输出的进制一致  ↓  不一致可能导致缓冲区不足，而溢出 */
    std::string out(lampz_to_str_len(z, 10), '\0');
    lamp_sz out_len = lampz_to_str(out.data(), out.capacity(), z, 10);
    /* 十进制输出---------------------------------------------------^ */

    /* 输出字符串依然为小端序，但输出的字符为小写字母 */
    /* 输出结果（字符串低位即数字低位），所以反向输出字符串 */
    for (size_t i = out_len; i-- != 0;) {
        std::cout << out[i];
    }
    std::cout << std::endl;

    /* 释放内存，z 本身为栈空间内存，无需释放，内部存在数组指针，调用此函数释放 */
    /* 调用此函数后，z 变为 NAN 空，效果等价于 __lampz_init(z); 此后也可以继续使用 */
    /* 当然如果当 z 离开作用域时，请务必调用此函数释放内存 */
    lampz_free(z);
    return 0;
}

```
