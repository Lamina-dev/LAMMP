# Lammp
Lamina的高精度计算库。

提供类似GMP的任意精度计算能力，同时支持或计划支持包括但不限于如开方、阶乘、组合数、素性检验、质因数分解等复杂的高精度计算。

## 编译

目前Lammp仅支持Windows平台，使用CMake构建，目前可支持MSVC，MinGW编译器，clang编译器暂无详细编译测试结果。

Lammp无标准库外的依赖，动态库LammpCore为纯c和汇编编写，无需其他依赖。

汇编代码目前仅支持采用MASM汇编器，目前有计划将汇编代码改写为C语言实现，作为fallback方案，以支持更多非x86-64平台。

## 架构

目前，非x86-64平台暂不支持，后续考虑将x64汇编函数使用C语言实现，以支持其他64位平台，32位平台暂无计划。

## 文档

目前，Lammp的文档暂无编写计划，在``doc/lammp/dev``下，已有部分文档描述，文档标准暂未制定，考虑到实用性，可能使用Markdown格式。

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
│       ├── impl/           # 内部实现头文件目录（仅供内部使用）
│       ├── version.h       # 版本头文件
│       └── .h              # 其他头文件
├── src/                    # 核心源代码根目录
│   └── lammp/              # 核心库源代码目录（对应include/lammp）
│       ├── CMakeLists.txt  # 核心动态库专属CMake（编译LammpCore）
│       ├── LammpCore.def   # MSVC专属定义动态库导出文件（MSVC编译时需要）
│       ├── global/         # 全局变量定义文件
│       ├── matrix/         # 矩阵运算子模块实现文件
│       ├── NumTh/          # 数论计算子模块实现文件
│       ├── secret/         # 密码学子模块实现文件
│       ├── mprand/         # 随机数生成子模块实现文件
│       ├── masm/.asm       # masm汇编源代码文件
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
#include "include/lammp/version.h"
#include <stdio.h>

int main() {
    printf("Hello, LAMMP! \n");
    printf("LAMMP version: %s\n", LAMMP_VERSION);
    printf("LAMMP alpha year: %s\n", LAMMP_ALPHA_YEAR);
    printf("LAMMP date: %s\n", LAMMP_DATE);
    printf("LAMMP compiler: %s\n", LAMMP_COMPILER);
    printf("======================================================\n");
    printf("LAMMP CopyRight: %s\n", LAMMP_COPYRIGHT);
    printf("======================================================\n");
    return 0;
}
```
