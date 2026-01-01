# 1. 指定CMake最低版本
cmake_minimum_required(VERSION 3.10)

# 2. 设置项目名称和编程语言
project(Lammp LANGUAGES CXX)


# 1. 设置默认构建类型为 Release（若用户未指定，自动使用 Release）
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Default build type: Release" FORCE)
endif()

# 2. 可选：限制用户只能选择合法的构建类型（避免无效输入）
set(VALID_BUILD_TYPES Debug Release RelWithDebInfo MinSizeRel)
if(NOT CMAKE_BUILD_TYPE IN_LIST VALID_BUILD_TYPES)
    message(FATAL_ERROR "Invalid CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}, valid types are: ${VALID_BUILD_TYPES}")
endif()

# 3. 针对不同构建类型，设置差异化编译选项（自动适配）
# Debug 模式：无优化（-O0）+ 生成调试信息（-g）+ 开启调试宏（-DDEBUG）
# Release 模式：最高优化（-O3）+ 关闭断言（-DNDEBUG）+ 你原本的优化选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-O0 -g -DDEBUG -Wall -Wextra -Wpedantic -fPIC -m64)
else() # Release/RelWithDebInfo/MinSizeRel 共用基础优化选项
    add_compile_options(-O3 -DNDEBUG -Wall -Wextra -Wpedantic -fPIC -m64)
    # RelWithDebInfo 额外保留调试信息
    if(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        add_compile_options(-g)
    endif()
    # MinSizeRel 优先最小体积（替换 -O3 为 -Os）
    if(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
        string(REPLACE "-O3" "-Os" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    endif()
endif()


# 3. 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 4. 全局编译优化选项（可选：全局生效，也可移到src/lammp中局部生效）
# add_compile_options(-Wall -Wextra -Wpedantic -O3 -fPIC -m64)

# 5. 全局链接选项（可选：全局生效，也可移到src/lammp中局部生效）
# add_link_options(-Wl,--enable-auto-import -Wl,--no-undefined)

# 6. 配置编译产物输出目录（指向dist/lammp，隔离其他库）
set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/dist/lammp)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR}/lib)

# 7. 包含公共头文件目录（全局生效，所有子模块均可引用）
include_directories(${CMAKE_SOURCE_DIR}/include)

# 8. 【核心修改】引入src/lammp目录（编译核心动态库）
# 这一步会自动执行src/lammp/CMakeLists.txt中的配置
add_subdirectory(src/lammp)

# 9. 可选：生成项目主可执行文件（若根目录有main.cpp）
if(EXISTS ${CMAKE_SOURCE_DIR}/main.cpp)
    add_executable(LammpMain ${CMAKE_SOURCE_DIR}/main.cpp)
    # 链接核心动态库（库名称LammpCore由src/lammp/CMakeLists.txt定义）
    target_link_libraries(LammpMain PRIVATE LammpCore)
endif()

# 10. 引入其他子模块（无修改）
add_subdirectory(benchmark/lammp)
add_subdirectory(example/lammp)
add_subdirectory(test/lammp)