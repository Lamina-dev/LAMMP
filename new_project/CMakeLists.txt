cmake_minimum_required (VERSION 3.10)

project ("LargeDynamicBitSetAndIntegerNumber")

set(CMAKE_BUILD_TYPE "Release")

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 静态链接 C++ 运行时库（MSVC 专用）
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# 设置默认的构建类型为 Debug，如果没有指定构建类型
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Debug' as none was specified.")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# 编译选项（保留调试信息配置）
if(MSVC)
    add_compile_options(/W4 /Zc:__cplusplus /utf-8)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")  
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(/O3 /DNDEBUG)
    else()
        add_compile_options(/Od /Zi /RTC1)
    endif()
else()
    # 原 GCC/Clang 编译选项保留，补充MinGW专属配置
    add_compile_options(-Wall -Wextra) # 可选：开启更多警告
    if(MINGW)
        # 核心：为MinGW生成调试信息，指定DWARF4格式（VTune友好）
        # Debug模式：无优化 + 最详细调试信息
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -gdwarf-4 -O0")
        # Release模式：保留O3优化 + 调试信息（性能分析用）
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g3 -gdwarf-4 -O3 -DNDEBUG")
        # 可选：禁用内联（解决内联函数被VTune识别为父函数的问题，按需开启）
        # set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-inline -fno-inline-small-functions")
        
    endif()
endif()

# 添加静态库
add_library(
    BigNumberSystem STATIC
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
target_link_libraries(Test PRIVATE BigNumberSystem)
# 仅对 MinGW (GCC on Windows) 生效
if(MINGW)
    # 静态链接 C++ 标准库和 GCC 运行时
    target_link_libraries(Test PRIVATE -static-libgcc -static-libstdc++)
    # 如果用了线程库，也静态链接
    if(CMAKE_THREAD_LIBS_INIT)
        target_link_libraries(Test PRIVATE -Wl,-Bstatic ${CMAKE_THREAD_LIBS_INIT})
    endif()
endif()

# 目标属性
set_target_properties(BigNumberSystem Test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    BUILD_WITH_INSTALL_RPATH OFF
)


message(STATUS "Build configuration complete: ${CMAKE_BUILD_TYPE}")
message(STATUS "MinGW debug info enabled: ${MINGW} (DWARF4)")