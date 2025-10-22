# CMakeList.txt: LargeDynamicBitSetAndIntegerNumber 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.10)

project ("LargeDynamicBitSetAndIntegerNumber")

#set(CMAKE_BUILD_TYPE Release)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置默认的构建类型为 Debug，如果没有指定构建类型
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Debug' as none was specified.")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
    # 设置可用的构建类型选项
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# 设置 Debug 和 Release 的编译选项
if(MSVC)
    add_compile_options(/W4 /Zc:__cplusplus /utf-8)
    # MSVC 的优化设置
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(/O3 /DNDEBUG)
    else()
        add_compile_options(/Od /Zi /RTC1)
    endif()
else()
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "11")
            message(FATAL_ERROR "GNU CXX compiler version is too small !")
        endif()
        
        # 为所有构建类型设置基本警告选项
        add_compile_options(-Wall -Wextra -fsigned-char -finput-charset=UTF-8 -fexec-charset=UTF-8)
        
        # 根据构建类型设置优化选项
        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            add_compile_options(-O3 -DNDEBUG)
            message(STATUS "Build type: Release - Optimization enabled")
        else()
            add_compile_options(-g -O0 -DDEBUG)
            message(STATUS "Build type: Debug - Debug symbols enabled")
        endif()
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # Clang 编译器的类似设置
        add_compile_options(-Wall -Wextra -fsigned-char)
        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            add_compile_options(-O3 -DNDEBUG)
        else()
            add_compile_options(-g -O0 -DDEBUG)
        endif()
    endif()
endif()

# 添加库
add_library(
    BigNumberSystem
    src/BigInt.hpp
    src/BigInt.cpp
    src/BigFrac.hpp
    src/BigFrac.cpp
)

# 添加可执行文件
add_executable(
    Test
    Test.cpp
    main.cpp
)

# 链接库
target_link_libraries(Test BigNumberSystem)

# 设置目标属性
set_target_properties(BigNumberSystem Test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# 在 Release 模式下也启用调试信息（可选）
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    #target_compile_options(Test PRIVATE -g)
    #target_compile_options(BigNumberSystem PRIVATE -g)
endif()

# 安装规则（可选）
install(TARGETS Test DESTINATION bin)
install(TARGETS BigNumberSystem DESTINATION lib)
install(FILES src/BigInt.hpp src/BigFrac.hpp DESTINATION include)

message(STATUS "Build configuration complete: ${CMAKE_BUILD_TYPE}")