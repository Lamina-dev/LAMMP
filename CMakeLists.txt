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

cmake_minimum_required(VERSION 3.10)
project(Lammp LANGUAGES C CXX)

option(USE_ASM "Enable x64 assembly with NASM (fallback to generic C if disabled)" ON)

# ===================== NASM 汇编器配置 =====================
if(USE_ASM AND WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 查找 NASM，结果存入缓存
    find_program(NASM_EXECUTABLE nasm
        PATHS
            "$ENV{LOCALAPPDATA}/bin/NASM"
            "C:/Users/Jimmy/AppData/Local/bin/NASM"
            "C:/Program Files/NASM"
            "C:/NASM"
        DOC "Path to NASM assembler"
    )
    if(NOT NASM_EXECUTABLE)
        message(FATAL_ERROR "NASM not found! Please install NASM or set the path manually.")
    endif()

    # 设置为缓存变量，使 CMake 在 enable_language 时使用
    set(CMAKE_ASM_NASM_COMPILER "${NASM_EXECUTABLE}" CACHE FILEPATH "NASM assembler" FORCE)

    enable_language(ASM_NASM)
    set(CMAKE_ASM_NASM_OBJECT_FORMAT win64)
endif()

# 设置默认构建类型为 Release
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Default build type: Release" FORCE)
endif()

# 限制合法的构建类型
set(VALID_BUILD_TYPES Debug Release RelWithDebInfo MinSizeRel)
if(CMAKE_BUILD_TYPE AND NOT CMAKE_BUILD_TYPE IN_LIST VALID_BUILD_TYPES)
    message(FATAL_ERROR "Invalid CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}, valid types are: ${VALID_BUILD_TYPES}")
endif()

# ===================== 通用编译器选项函数 =====================
function(set_common_compiler_options target)
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
        target_compile_definitions(${target} PRIVATE
            $<$<AND:$<COMPILE_LANGUAGE:ASM_NASM>,$<PLATFORM_ID:Windows>>:LAMMP_ASM_WIN>
        )
        if(CMAKE_C_COMPILER_ID MATCHES "GNU")
            target_compile_options(${target} PRIVATE 
                $<$<COMPILE_LANGUAGE:C>: -mbmi2 >
                $<$<COMPILE_LANGUAGE:CXX>: -mbmi2 >
            )
        elseif(CMAKE_C_COMPILER_ID MATCHES "MSVC")
            target_compile_options(${target} PRIVATE 
                $<$<COMPILE_LANGUAGE:C>: /arch:AVX2 >
                $<$<COMPILE_LANGUAGE:CXX>: /arch:AVX2 > 
            )
        endif()
    endif()
    if(CMAKE_C_COMPILER_ID MATCHES "GNU")
        target_compile_options(${target} PRIVATE
            $<$<COMPILE_LANGUAGE:C>:
                -Wall -Wextra -Wpedantic -fPIC -m64 -mcmodel=large
                $<$<CONFIG:Debug>:-O0 -g -DDEBUG>
                $<$<CONFIG:Release>:-O3 -g0 -fno-math-errno -DNDEBUG>
                $<$<CONFIG:MinSizeRel>:-Os -DNDEBUG>
                $<$<CONFIG:RelWithDebInfo>:-O2 -g -DNDEBUG>
            >
            $<$<COMPILE_LANGUAGE:CXX>:
                -Wall -Wextra -Wpedantic -fPIC -m64 -mcmodel=large
                $<$<CONFIG:Debug>:-O0 -g -DDEBUG>
                $<$<CONFIG:Release>:-O3 -g0 -fno-math-errno -DNDEBUG>
                $<$<CONFIG:MinSizeRel>:-Os -DNDEBUG>
                $<$<CONFIG:RelWithDebInfo>:-O2 -g -DNDEBUG>
            >
        )
        target_link_options(${target} PRIVATE
            -m64 -Wl,--enable-auto-import -Wl,--no-undefined
            $<$<CONFIG:Release>:-s -Wl,--strip-all> 
            $<$<CONFIG:MinSizeRel>:-s -Wl,--strip-all>
            $<$<CONFIG:Debug>:> 
            $<$<CONFIG:RelWithDebInfo>:>
        )
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

# ===================== 输出目录配置 =====================
set(OUTPUT_ROOT ${CMAKE_SOURCE_DIR}/dist/lammp)

foreach(CONFIG_TYPE Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER ${CONFIG_TYPE} CONFIG_TYPE_UPPER)
    # DLL/可执行文件输出目录
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} "${OUTPUT_ROOT}/bin/${CONFIG_TYPE}")
    # 静态库/导入库输出目录
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} "${OUTPUT_ROOT}/lib/${CONFIG_TYPE}")
    # MinGW的动态库输出目录
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} "${OUTPUT_ROOT}/lib/${CONFIG_TYPE}")
endforeach()

# 单配置编译器（MinGW）的默认输出
if(NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_ROOT}/bin/${CMAKE_BUILD_TYPE}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_ROOT}/lib/${CMAKE_BUILD_TYPE}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_ROOT}/lib/${CMAKE_BUILD_TYPE}")
endif()

# ===================== 构建子模块 =====================
add_subdirectory(src/lammp)

if(EXISTS ${CMAKE_SOURCE_DIR}/main.c)
    add_executable(LammpMain ${CMAKE_SOURCE_DIR}/main.c)
    target_link_libraries(LammpMain PRIVATE LammpCore)
    set_common_compiler_options(LammpMain)
endif()

add_subdirectory(test/lammp)
add_subdirectory(example/lammp)
add_subdirectory(benchmark/lammp)