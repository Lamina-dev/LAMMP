# [LAMMP]
# Copyright (C) [2025-2026] [HJimmyK(Jericho Knox)]
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.15)
project(Lammp LANGUAGES C CXX)

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
    endif()

    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE
            $<$<COMPILE_LANGUAGE:C>:
                -Wall -Wextra -Wpedantic -m64 -mcmodel=large
                $<$<NOT:$<PLATFORM_ID:Windows>>:-fPIC>
                $<$<CONFIG:Debug>:-O0 -g -DDEBUG>
                $<$<CONFIG:Release>:-O3 -g0 -fno-math-errno -DNDEBUG>
                $<$<CONFIG:MinSizeRel>:-Os -DNDEBUG>
                $<$<CONFIG:RelWithDebInfo>:-O2 -g -DNDEBUG>
            >
            $<$<COMPILE_LANGUAGE:CXX>:
                -Wall -Wextra -Wpedantic -m64 -mcmodel=large
                $<$<NOT:$<PLATFORM_ID:Windows>>:-fPIC>
                $<$<CONFIG:Debug>:-O0 -g -DDEBUG>
                $<$<CONFIG:Release>:-O3 -g0 -fno-math-errno -DNDEBUG>
                $<$<CONFIG:MinSizeRel>:-Os -DNDEBUG>
                $<$<CONFIG:RelWithDebInfo>:-O2 -g -DNDEBUG>
            >
        )

        if(CMAKE_C_COMPILER_ID MATCHES "GNU")
            target_link_options(${target} PRIVATE
                -m64 -Wl,--no-undefined
                $<$<CONFIG:Release>:-s -Wl,--strip-all>
                $<$<CONFIG:MinSizeRel>:-s -Wl,--strip-all>
            )
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            if(TARGET_SYSTEM STREQUAL "WIN")
            else()
                target_link_options(${target} PRIVATE
                    $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:-s -Wl,--strip-all>
                )
            endif()
        endif()

    elseif(CMAKE_C_COMPILER_ID MATCHES "MSVC")
        target_compile_options(${target} PRIVATE
            $<$<COMPILE_LANGUAGE:C>:
                /MP /W4 /DWIN64 /D_CRT_SECURE_NO_WARNINGS
                $<$<CONFIG:Debug>:/Od /Zi /DDEBUG /MDd>
                $<$<CONFIG:Release>:/Ox /DNDEBUG /MD>
                $<$<CONFIG:MinSizeRel>:/Os /DNDEBUG /MD>
                $<$<CONFIG:RelWithDebInfo>:/O2 /Zi /DNDEBUG /MD>
            >
            $<$<COMPILE_LANGUAGE:CXX>:
                /MP /W4 /DWIN64
                $<$<CONFIG:Debug>:/Od /Zi /DDEBUG /MDd>
                $<$<CONFIG:Release>:/Ox /DNDEBUG /MD>
                $<$<CONFIG:MinSizeRel>:/Os /DNDEBUG /MD>
                $<$<CONFIG:RelWithDebInfo>:/O2 /Zi /DNDEBUG /MD>
            >
        )

        target_link_options(${target} PRIVATE
            /MACHINE:X64 /DYNAMICBASE /NXCOMPAT /SUBSYSTEM:CONSOLE
        )
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
message(STATUS "Target system: ${TARGET_SYSTEM}")
message(STATUS "Build type:    ${CMAKE_BUILD_TYPE}")
message(STATUS "ASM support:   ${USE_ASM}")
message(STATUS "Output dir:    ${OUTPUT_ROOT}")
message(STATUS "=====================================")