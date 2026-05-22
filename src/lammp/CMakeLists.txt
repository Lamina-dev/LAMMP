# LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
# This file is part of lammp, under the GNU LGPL v2 license.
# See LICENSE in the project root for the full license text.

file(GLOB_RECURSE LAMMP_CORE_SRCS
    LIST_DIRECTORIES FALSE
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
)

if(USE_ASM AND CMAKE_SIZEOF_VOID_P EQUAL 8)
    
    # 启用汇编时，需要使用.def文件导出汇编实现的函数
    # 未启用时，纯c实现的函数会自动导出 
    set(DEF_FILE ${CMAKE_CURRENT_SOURCE_DIR}/asm/lammp_asm.def)
    if(TARGET_SYSTEM STREQUAL "WIN")
        list(APPEND LAMMP_CORE_SRCS ${DEF_FILE})
    endif()
    # 过滤掉 generic 目录下的源文件
    list(FILTER LAMMP_CORE_SRCS EXCLUDE REGEX ".*/generic/.*")

    # 收集汇编文件
    file(GLOB ASM_SRCS
        LIST_DIRECTORIES FALSE
        ${CMAKE_CURRENT_SOURCE_DIR}/asm/x64/*.asm
    )

    if(NOT ASM_SRCS)
        message(FATAL_ERROR "USE_ASM is enabled but no assembly files found!")
    endif()

    list(APPEND LAMMP_CORE_SRCS ${ASM_SRCS})
    message(STATUS "LammpCore: Using ASM (Windows x64)")
else()
    # 不启用汇编则使用纯 C 通用实现
    message(STATUS "LammpCore: Using generic C implementation")
endif()

add_library(LammpCore SHARED ${LAMMP_CORE_SRCS})
add_library(Lammp::Core ALIAS LammpCore)

if(TARGET_SYSTEM STREQUAL "WIN")
    target_compile_definitions(LammpCore PRIVATE LAMMP_CORE_EXPORTS)
endif()

set_target_properties(LammpCore PROPERTIES
    OUTPUT_NAME "LammpCore"
    VERSION 4.2.0
    SOVERSION 1
    PDB_OUTPUT_DIRECTORY "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}"
)

lammp_target_setup(LammpCore)

message(STATUS "LammpCore: configured successfully (shared library)")