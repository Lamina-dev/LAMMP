# Lammp
Lamina的高精度计算库。

提供接近GMP的任意精度计算能力，在可以支持汇编的情况下，LAMMP和GMP的性能差距通常不超过5%。在基础算子中，LAMMP的不平衡乘法优于GMP，LAMMP通常快于GMP大约40%-10%，具体性能差距与操作数长度有关。有关具体地性能差异，可以自行测量。

LAMMP同时支持或计划支持包括但不限于如开方、阶乘、组合数、素性检验、质因数分解等复杂的高精度计算。很多功能正在开发中。

## 编译

目前Lammp使用CMake构建，目前可支持MSVC，MinGW/Gcc，和clang编译器。受限于测试平台，在linux系统中暂无详细的编译测试。Lammp无标准库外的依赖，动态库LammpCore为纯c和汇编编写，无需其他依赖。

汇编使用nasm汇编器，设计目标可以跨系统（windows和linux）。对于非x64平台，未来仅会针对arm64架构再进行额外的汇编编写。同时目前已有generic实现，在``src/lammp/generic/``下。可以设置编译选项``--DUSE_ASM=ON``，请注意，对于非x64架构，开启此选项会导致编译失败。

请注意，由于使用了BMI2指令集，所以x64架构的CPU最低要求 Intel Haswell及以上或者 AMD Zen 及以上。

## 接口与调用说明

目前，Lammp的接口主要在``include/lammp/``目录下，分为两个系列：

+ 通用算子系列：
  这部分模块为核心算子，为尽可能高效，通常均有极为严格的调用限制。
  - ``lmmp.h``：Lammp全局资源管理模块，包含如初始化、释放线程局部资源，abort回调函数，内存管理等功能。
  - ``lmmpn.h``：多精度整数运算子模块，包含如加减乘除、取模、移位、比较、开方等高精度计算。
  - ``numth.h``：数论运算子模块，包含如幂次方、逆元、GCD、阶乘、组合数、素性检验等高精度计算。
  - ``mprand.h``：随机数生成子模块，包含生成高精度随机数生成算法。
  - ``secret.h``：加密模块，包含如hash、加密解密（未实现）等高精度计算。
+ 标准接口系列：
  这部分模块均未实现，计划采用不透明结构体的方式，提供更高级的接口。

## 目录结构

```
LAMMP/                      # 项目根目录
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
│       ├── impl/           # 内部实现头文件目录（仅供内部使用）
│       ├── version.h       # 版本头文件
│       └── .h              # 其他头文件
├── src/                    # 核心源代码根目录
│   └── lammp/              # 核心库源代码目录（对应include/lammp）
│       ├── CMakeLists.txt  # 核心动态库专属CMake（编译LammpCore）
│       ├── global/         # 全局变量定义文件
│       ├── NumTh/          # 数论计算子模块实现文件
│       ├── secret/         # 密码学子模块实现文件
│       ├── mprand/         # 随机数生成子模块实现文件
│       ├── asm/xx/.asm     # nasm汇编源代码文件（按照架构分类）
│       └── .c              # 实现文件
├── benchmark/              # 基准测试根目录
│   └── lammp/              # Lammp项目基准测试目录
│       ├── CMakeLists.txt  # 基准测试CMake配置
│       ├── include/        # 基准测试私有头文件（仅测试内部使用）
│       ├── src/            # 基准测试源代码目录
│       └── main.cpp        # 基准测试主程序main()
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
#include <stdio.h>
#include "include/lammp/numth.h"

int main() {
    lmmp_global_init(); // 初始化Lammp（单线程）全局资源

    uint n = 10000;
    printf("calculating factorial of %d\n", n);
    mp_bitcnt_t bits;
    mp_size_t len = lmmp_factorial_size_(n, &bits); // 获取阶乘缓冲区大小
    mp_ptr dst = (mp_ptr)lmmp_alloc(len * sizeof(mp_limb_t)); // 分配阶乘缓冲区
    len = lmmp_factorial_(dst, bits, len, n); // 计算阶乘
    printf("completed.\n");

    printf("result: %llx ... %llx\n", dst[len - 1], dst[0]);

    lmmp_free(dst);
    lmmp_global_deinit(); // 释放Lammp（单线程）全局资源
}
```
