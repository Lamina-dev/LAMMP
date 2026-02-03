# 递归收集源代码
file(GLOB_RECURSE LAMMP_CORE_SRCS 
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
    LIST_DIRECTORIES FALSE
)

if(CMAKE_C_COMPILER_ID MATCHES "MSVC")
    file(GLOB MASM_SRCS 
        ${CMAKE_CURRENT_SOURCE_DIR}/masm/*.asm
        LIST_DIRECTORIES FALSE
    )
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU")
    file(GLOB MASM_SRCS 
        ${CMAKE_CURRENT_SOURCE_DIR}/masm/*.asm
        LIST_DIRECTORIES FALSE
    )
endif()

# 仅在MASM文件存在时添加
if(MASM_SRCS)
    list(APPEND LAMMP_CORE_SRCS ${MASM_SRCS})
endif()

if(NOT TARGET LammpCore)
    add_library(LammpCore SHARED ${LAMMP_CORE_SRCS})
endif()

# 动态库导出配置
target_compile_definitions(LammpCore PRIVATE
    $<$<BOOL:${WIN32}>:LAMMP_CORE_EXPORTS>
    $<$<BOOL:${WIN32}>:_CRT_SECURE_NO_WARNINGS>
)

# 设置目标属性
if(TARGET LammpCore)
    set_target_properties(LammpCore PROPERTIES
        OUTPUT_NAME "LammpCore"
        VERSION 2.0.1
        SOVERSION 1
        # PDB输出路径
        PDB_OUTPUT_DIRECTORY "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}"
        # 导入库输出路径
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}"
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}"
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}"
        # DLL输出路径
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    )

    # MinGW下设置DLL后缀
    if(CMAKE_C_COMPILER_ID MATCHES "GNU")
        set_target_properties(LammpCore PROPERTIES
            SUFFIX ".dll"
        )
    endif()

    # MSVC下使用DEF文件
    if(CMAKE_C_COMPILER_ID MATCHES "MSVC")
        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/LammpCore.def)
            target_link_options(LammpCore PRIVATE 
                "/DEF:${CMAKE_CURRENT_SOURCE_DIR}/LammpCore.def"
                "/IMPLIB:${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/LammpCore.lib"
            )
        else()
            message(WARNING "LammpCore.def not found! MSVC import library may be incomplete.")
        endif()
    endif()

    # 头文件包含
    target_include_directories(LammpCore
        PUBLIC
            ${CMAKE_SOURCE_DIR}/include
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/masm
    )

    # 设置编译器选项
    set_common_compiler_options(LammpCore)

    # MSVC下复制DEF文件
    if(CMAKE_C_COMPILER_ID MATCHES "MSVC" AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/LammpCore.def)
        add_custom_command(TARGET LammpCore POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/LammpCore.def
            ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/LammpCore.def
            COMMENT "Copying LammpCore.def to output directory"
        )
    endif()
endif()