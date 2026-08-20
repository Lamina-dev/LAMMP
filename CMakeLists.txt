#
#  Copyright (C) 2026 HJimmyK(Jericho Knox)
#
#  This file is part of LAMMP.
#
#  LAMMP is free software: you can redistribute it and/or modify it under
#  the terms of the GNU Lesser General Public License (LGPL) as published
#   by the Free Software Foundation; either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed WITHOUT ANY WARRANTY. 
#
#  See <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.29)
project(LAMMP LANGUAGES C CXX)

# ==============================================================================
# 0. 工具链限制：仅支持 GNU/Clang 风格工具链
# ==============================================================================
if(MSVC OR CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    # 原生 MSVC 与 clang-cl 均已停止支持。
    # Windows 下请使用 MinGW GCC 或 GNU 驱动的 clang。
    message(FATAL_ERROR "MSVC/clang-cl toolchain is not supported. Use MinGW GCC or GNU-driver clang on Windows.")
endif()

# ==============================================================================
# 1. 可配置选项
# ==============================================================================
set(LAMMP_X86_64 FALSE)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64|x64|X64)$")
    set(LAMMP_X86_64 TRUE)
endif()

# 汇编默认关闭：需要 x86_64 + NASM 时显式 -DUSE_ASM=ON 开启。
option(USE_ASM "Enable x64 assembly optimization (require NASM)" OFF)

# 测试与基准目标默认开启，可独立关闭。
option(LAMMP_BUILD_TESTS "Build LAMMP unit tests" ON)
option(LAMMP_BUILD_BENCHMARKS "Build LAMMP benchmarks" ON)

# 目标系统：自动识别，也可手动指定 WIN | LIN | MAC
set(TARGET_SYSTEM "AUTO" CACHE STRING "Target system: AUTO | WIN | LIN | MAC")
set_property(CACHE TARGET_SYSTEM PROPERTY STRINGS AUTO WIN LIN MAC)

# 构建类型检查
set(VALID_BUILD_TYPES Debug Release RelWithDebInfo MinSizeRel)
if(CMAKE_BUILD_TYPE AND NOT CMAKE_BUILD_TYPE IN_LIST VALID_BUILD_TYPES)
    message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
endif()

# ==============================================================================
# 2. 目标系统
# ==============================================================================
if(TARGET_SYSTEM STREQUAL "AUTO")
    if(WIN32)
        set(TARGET_SYSTEM "WIN")
    elseif(APPLE)
        set(TARGET_SYSTEM "MAC")
    else()
        set(TARGET_SYSTEM "LIN")
    endif()
endif()

string(TOUPPER ${TARGET_SYSTEM} TARGET_SYSTEM_UPPER)
message(STATUS "Target system: ${TARGET_SYSTEM_UPPER}")

# ==============================================================================
# 3. 汇编支持
# ==============================================================================
if(USE_ASM AND NOT LAMMP_X86_64)
    message(STATUS "ASM support requested on non-x86_64 architecture; falling back to generic C implementation")
    set(USE_ASM OFF CACHE BOOL "Enable x64 assembly optimization (require NASM)" FORCE)
endif()

if(USE_ASM AND CMAKE_SIZEOF_VOID_P EQUAL 8)
    find_program(NASM_EXECUTABLE nasm
        PATHS
            "$ENV{LOCALAPPDATA}/bin/NASM"
            "C:/Users/Jimmy/AppData/Local/bin/NASM"
            "C:/Program Files/NASM"
            "C:/NASM"
    )
    if(NOT NASM_EXECUTABLE)
        message(FATAL_ERROR "NASM not found! Please install NASM or disable USE_ASM.")
    endif()

    set(CMAKE_ASM_NASM_COMPILER ${NASM_EXECUTABLE} CACHE FILEPATH "" FORCE)
    enable_language(ASM_NASM)
    if(TARGET_SYSTEM STREQUAL "WIN")
        set(CMAKE_ASM_NASM_OBJECT_FORMAT win64)
    elseif(TARGET_SYSTEM STREQUAL "LIN")
        set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
    elseif(TARGET_SYSTEM STREQUAL "MAC")
        message(FATAL_ERROR "LAMMP assembly is not supported on macOS; disable USE_ASM.")
    else()
        message(FATAL_ERROR "Unsupported TARGET_SYSTEM: ${TARGET_SYSTEM}")
    endif()
    message(STATUS "ASM support: ENABLED (using NASM: ${NASM_EXECUTABLE})")
else()
    set(USE_ASM OFF)
    message(STATUS "ASM support: DISABLED")
endif()

# ==============================================================================
# 4. 默认构建类型
# ==============================================================================
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# ==============================================================================
# 5. 通用编译选项函数
# ==============================================================================
function(lammp_target_setup target)

    set_target_properties(${target} PROPERTIES

        C_STANDARD 11

        C_STANDARD_REQUIRED ON

        C_EXTENSIONS OFF

        CXX_STANDARD 17

        CXX_STANDARD_REQUIRED ON

        CXX_EXTENSIONS OFF

    )



    # 仅保留 GNU/Clang 风格编译选项。MSVC/clang-cl 已在配置阶段被拒绝。

    target_compile_options(${target} PRIVATE
        $<$<COMPILE_LANGUAGE:C>:
            -Wall -Wextra -Wpedantic
            $<$<NOT:$<PLATFORM_ID:Windows>>:-fPIC>
            $<$<CONFIG:Debug>:-O0 -g -DDEBUG>
            $<$<CONFIG:Release>:-O3 -g0 -fno-math-errno -DNDEBUG>
            $<$<CONFIG:MinSizeRel>:-Os -DNDEBUG>
            $<$<CONFIG:RelWithDebInfo>:-O2 -g -DNDEBUG>
        >
        $<$<COMPILE_LANGUAGE:CXX>:
            -Wall -Wextra -Wpedantic
            $<$<NOT:$<PLATFORM_ID:Windows>>:-fPIC>
            $<$<CONFIG:Debug>:-O0 -g -DDEBUG>
            $<$<CONFIG:Release>:-O3 -g0 -fno-math-errno -DNDEBUG>
            $<$<CONFIG:MinSizeRel>:-Os -DNDEBUG>
            $<$<CONFIG:RelWithDebInfo>:-O2 -g -DNDEBUG>
        >
    )
    message(STATUS "Compile options: GNU/Clang standard options set.")

    if(USE_ASM)
        target_compile_definitions(${target} PRIVATE USE_ASM)
        if(TARGET_SYSTEM STREQUAL "WIN")
            target_compile_definitions(${target} PRIVATE LAMMP_ASM_WIN)
        endif()
        target_compile_options(${target} PRIVATE
            $<$<COMPILE_LANGUAGE:C>: -mbmi2>
            $<$<COMPILE_LANGUAGE:CXX>: -mbmi2>
        )
    endif()

    if(TARGET_SYSTEM STREQUAL "WIN")
        target_compile_definitions(${target} PRIVATE LAMMP_WINDOWS)
    elseif(TARGET_SYSTEM STREQUAL "LIN")
        target_compile_definitions(${target} PRIVATE LAMMP_LINUX)
        target_compile_options(${target} PRIVATE
            $<$<COMPILE_LANGUAGE:C>: -fno-plt>
            $<$<COMPILE_LANGUAGE:CXX>: -fno-plt>
        )
    elseif(TARGET_SYSTEM STREQUAL "MAC")
        target_compile_definitions(${target} PRIVATE LAMMP_MACOS)
    endif()

    if(APPLE)
        target_link_options(${target} PRIVATE
            $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:-Wl,-dead_strip>
        )
        message(STATUS "Linker options: Apple ld standard options set.")
    elseif(MINGW OR TARGET_SYSTEM STREQUAL "LIN")
        # MinGW 或 Linux 使用 GNU ld 风格
        target_link_options(${target} PRIVATE
            -Wl,--no-undefined
            $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:
                -Wl,--strip-all
            >
        )
        message(STATUS "Linker options: GNU ld standard options set.")
    else()
        # Windows 上 GNU 驱动的 clang 等工具链，不附加平台特有链接选项。
        message(STATUS "Linker options: default for this toolchain.")
    endif()

endfunction()

# ==============================================================================
# 6. 输出目录
# ==============================================================================
set(OUTPUT_ROOT ${CMAKE_SOURCE_DIR}/dist/lammp)
foreach(TYPE RUNTIME ARCHIVE LIBRARY)
    set(CMAKE_${TYPE}_OUTPUT_DIRECTORY ${OUTPUT_ROOT}/bin)
    set(CMAKE_${TYPE}_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_ROOT}/bin/debug)
    set(CMAKE_${TYPE}_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_ROOT}/bin/release)
    set(CMAKE_${TYPE}_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OUTPUT_ROOT}/bin/relwithdebinfo)
    set(CMAKE_${TYPE}_OUTPUT_DIRECTORY_MINSIZEREL ${OUTPUT_ROOT}/bin/minsizerel)
endforeach()

# ==============================================================================
# 7. 构建子模块
# ==============================================================================
add_subdirectory(src/lammp)

if(EXISTS ${CMAKE_SOURCE_DIR}/main.c)
    add_executable(LammpMain main.c)
    target_link_libraries(LammpMain PRIVATE LammpCore)
    lammp_target_setup(LammpMain)
endif()

if(LAMMP_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test/lammp)
endif()

add_subdirectory(example/lammp)

if(LAMMP_BUILD_BENCHMARKS)
    add_subdirectory(benchmark/lammp)
endif()

# ==============================================================================
# 8. 最终信息输出
# ==============================================================================
message(STATUS "=====================================")
message(STATUS "Lammp configure summary")
message(STATUS "=====================================")
message(STATUS "Compiler:      ${CMAKE_C_COMPILER_ID}")
message(STATUS "Linker:        ${CMAKE_C_COMPILER_LINKER_ID}")
message(STATUS "Target system: ${TARGET_SYSTEM}")
message(STATUS "Build type:    ${CMAKE_BUILD_TYPE}")
message(STATUS "ASM support:   ${USE_ASM}")
message(STATUS "Output dir:    ${OUTPUT_ROOT}")
message(STATUS "=====================================")
