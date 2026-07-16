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
# 1. 可配置选项
# ==============================================================================
option(USE_ASM "Enable x64 assembly optimization (require NASM)" ON)

# 目标系统：自动识别，也可手动指定 WIN | LIN
set(TARGET_SYSTEM "AUTO" CACHE STRING "Target system: AUTO | WIN | LIN")
set_property(CACHE TARGET_SYSTEM PROPERTY STRINGS AUTO WIN LIN)

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
    else()
        set(TARGET_SYSTEM "LIN")
    endif()
endif()

string(TOUPPER ${TARGET_SYSTEM} TARGET_SYSTEM_UPPER)
message(STATUS "Target system: ${TARGET_SYSTEM_UPPER}")

# ==============================================================================
# 3. 汇编支持
# ==============================================================================
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

    if(USE_ASM)
        target_compile_definitions(${target} PRIVATE USE_ASM)
        if(TARGET_SYSTEM STREQUAL "WIN")
            target_compile_definitions(${target} PRIVATE LAMMP_ASM_WIN)
        endif()

        if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(${target} PRIVATE
                $<$<COMPILE_LANGUAGE:C>: -mbmi2>
                $<$<COMPILE_LANGUAGE:CXX>: -mbmi2>
            )
        elseif(CMAKE_C_COMPILER_ID MATCHES "MSVC")
            target_compile_options(${target} PRIVATE
                $<$<COMPILE_LANGUAGE:C>: /arch:AVX2>
                $<$<COMPILE_LANGUAGE:CXX>: /arch:AVX2>
            )
        endif()
    endif()

    if(TARGET_SYSTEM STREQUAL "WIN")
        target_compile_definitions(${target} PRIVATE LAMMP_WINDOWS)
    elseif(TARGET_SYSTEM STREQUAL "LIN")
        target_compile_definitions(${target} PRIVATE LAMMP_LINUX)
    endif()

    if(MSVC)
        if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
            set(MP_FLAG "/MP")
        else()
            set(MP_FLAG "")
        endif()

        target_compile_options(${target} PRIVATE
            $<$<COMPILE_LANGUAGE:C>:
                ${MP_FLAG} /W4 /DWIN64 /D_CRT_SECURE_NO_WARNINGS
                /wd4305 /wd4244
                $<$<CONFIG:Debug>:/Od /Zi /DDEBUG /MDd>
                $<$<CONFIG:Release>:/Ox /DNDEBUG /MD>
                $<$<CONFIG:MinSizeRel>:/Os /DNDEBUG /MD>
                $<$<CONFIG:RelWithDebInfo>:/O2 /Zi /DNDEBUG /MD>
            >
            $<$<COMPILE_LANGUAGE:CXX>:
                ${MP_FLAG} /W4 /DWIN64 /D_CRT_SECURE_NO_WARNINGS
                /wd4305 /wd4244
                $<$<CONFIG:Debug>:/Od /Zi /DDEBUG /MDd>
                $<$<CONFIG:Release>:/Ox /DNDEBUG /MD>
                $<$<CONFIG:MinSizeRel>:/Os /DNDEBUG /MD>
                $<$<CONFIG:RelWithDebInfo>:/O2 /Zi /DNDEBUG /MD>
            >
        )
        message(STATUS "Compile options: MSVC standard options set.")
    else()
        target_compile_options(${target} PRIVATE
            $<$<COMPILE_LANGUAGE:C>:
                -Wall -Wextra -Wpedantic -m64
                $<$<NOT:$<PLATFORM_ID:Windows>>:-fPIC>
                $<$<CONFIG:Debug>:-O0 -g -DDEBUG>
                $<$<CONFIG:Release>:-O3 -g0 -fno-math-errno -DNDEBUG>
                $<$<CONFIG:MinSizeRel>:-Os -DNDEBUG>
                $<$<CONFIG:RelWithDebInfo>:-O2 -g -DNDEBUG>
            >
            $<$<COMPILE_LANGUAGE:CXX>:
                -Wall -Wextra -Wpedantic -m64
                $<$<NOT:$<PLATFORM_ID:Windows>>:-fPIC>
                $<$<CONFIG:Debug>:-O0 -g -DDEBUG>
                $<$<CONFIG:Release>:-O3 -g0 -fno-math-errno -DNDEBUG>
                $<$<CONFIG:MinSizeRel>:-Os -DNDEBUG>
                $<$<CONFIG:RelWithDebInfo>:-O2 -g -DNDEBUG>
            >
        )
        message(STATUS "Compile options: GNU standard options set.")
    endif()

    if(MSVC)
        # MSVC 兼容编译器（原生 MSVC、clang-cl、Intel 等）
        target_link_options(${target} PRIVATE
            /DYNAMICBASE
            /NXCOMPAT
            /SUBSYSTEM:CONSOLE
            $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:/OPT:REF /OPT:ICF>
        )
        message(STATUS "Linker options: msvc standard options set.")
    elseif(TARGET_SYSTEM STREQUAL "WIN" AND NOT MINGW)
        # Windows 上使用 clang（GNU 驱动），目标为 MSVC ABI
        # 使用 -Wl, 将 MSVC 选项原样转发给链接器
        target_link_options(${target} PRIVATE
            -Wl,/DYNAMICBASE
            -Wl,/NXCOMPAT
            -Wl,/SUBSYSTEM:CONSOLE
            $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:
                -Wl,/OPT:REF -Wl,/OPT:ICF
            >
        )
        message(STATUS "Linker options: clang standard options set.")
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
        # 其他平台（如 macOS）需要单独处理
        message(STATUS "Warning: Unknown platform. No link options set for this platform.")
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

add_subdirectory(test/lammp)
add_subdirectory(example/lammp)
add_subdirectory(benchmark/lammp)

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