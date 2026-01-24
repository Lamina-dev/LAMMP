# 递归收集源代码
file(GLOB_RECURSE LAMMP_CORE_SRCS 
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
)

if(MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8)
    file(GLOB_RECURSE LAMMP_CORE_ASM_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.asm)
    list(APPEND LAMMP_CORE_SRCS ${LAMMP_CORE_ASM_SRCS})
    # Exclude generic implementation when using MASM
    list(FILTER LAMMP_CORE_SRCS EXCLUDE REGEX "generic_c.c$")
endif()

# 生成动态库
add_library(LammpCore SHARED ${LAMMP_CORE_SRCS})

# 设置动态库属性
set_target_properties(LammpCore PROPERTIES
    OUTPUT_NAME "LammpCore"
    VERSION 2.0.1
    SOVERSION 1
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
# ===================== 局部编译/链接选项 =====================
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(LammpCore PRIVATE
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wall>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wextra>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wpedantic>
        $<$<COMPILE_LANGUAGE:C,CXX>:-O3>
        $<$<COMPILE_LANGUAGE:C,CXX>:-ffast-math>
        $<$<COMPILE_LANGUAGE:C,CXX>:-fPIC>
        $<$<COMPILE_LANGUAGE:C,CXX>:-m64>
    )
    target_link_options(LammpCore PRIVATE
        -Wl,--enable-auto-import -Wl,--no-undefined
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(LammpCore PRIVATE
        $<$<COMPILE_LANGUAGE:C,CXX>:/W4>
        $<$<COMPILE_LANGUAGE:C,CXX>:/MP>
        $<$<COMPILE_LANGUAGE:C,CXX>:/Ox>
        $<$<COMPILE_LANGUAGE:C,CXX>:/fp:fast>
    )
    target_link_options(LammpCore PRIVATE
        /MACHINE:X64 /DYNAMICBASE
    )
endif()

if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_compile_options(LammpCore PRIVATE
        $<$<COMPILE_LANGUAGE:ASM_MASM>:/nologo>  
    )
endif()