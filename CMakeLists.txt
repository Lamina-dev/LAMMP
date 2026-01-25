cmake_minimum_required(VERSION 3.10)
project(Lammp LANGUAGES C CXX)

# 延迟启用汇编语言，根据编译器类型选择正确的汇编器
if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
    if(CMAKE_C_COMPILER_ID MATCHES "MSVC")
        enable_language(ASM_MASM)
    elseif(CMAKE_C_COMPILER_ID MATCHES "GNU")
        # 对于MinGW，我们仍然使用ASM_MASM语言，但手动配置ml64编译器
        enable_language(ASM_MASM)
    endif()
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

# ===================== MASM 汇编器配置（跨编译器适配） =====================
if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
    if(CMAKE_C_COMPILER_ID MATCHES "MSVC")
        # MSVC原生MASM配置
        set(CMAKE_ASM_MASM_FLAGS "/c /nologo /DWIN64")
        set(CMAKE_ASM_MASM_FLAGS_INIT "${CMAKE_ASM_MASM_FLAGS}")
        set(CMAKE_ASM_MASM_COMPILE_OBJECT "<CMAKE_ASM_MASM_COMPILER> <FLAGS> /Fo<OBJECT> <SOURCE>")
    elseif(CMAKE_C_COMPILER_ID MATCHES "GNU")
        # MinGW下使用MASM (ml64.exe) 配置
        # 查找ml64.exe（MASM64位编译器）
        find_program(ML64_EXECUTABLE
            NAMES ml64.exe
            PATHS 
                "$ENV{VSINSTALLDIR}/bin/Hostx64/x64"
                "$ENV{ProgramFiles}/Microsoft Visual Studio/*/BuildTools/bin/Hostx64/x64"
                "$ENV{ProgramFiles}/Microsoft Visual Studio/*/Community/bin/Hostx64/x64"
                "${PROGRAMFILES}/Microsoft Visual Studio/*/BuildTools/bin/Hostx64/x64"
                "${PROGRAMFILES}/Microsoft Visual Studio/*/Community/bin/Hostx64/x64"
                "D:/vs2022/IDE/VC/Tools/MSVC/14.44.35207/bin/Hostx64/x64"
            REQUIRED
        )
        
        # 设置MASM编译器
        set(CMAKE_ASM_MASM_COMPILER ${ML64_EXECUTABLE})
        # MASM编译参数
        set(CMAKE_ASM_MASM_FLAGS "/c /nologo /DWIN64 /Zi")
        set(CMAKE_ASM_MASM_FLAGS_INIT "${CMAKE_ASM_MASM_FLAGS}")
        # 自定义编译规则：调用ml64.exe编译汇编文件
        set(CMAKE_ASM_MASM_COMPILE_OBJECT 
            "<CMAKE_ASM_MASM_COMPILER> <FLAGS> /Fo<OBJECT> <SOURCE>"
        )
        
        # 确保汇编文件被正确识别
        file(GLOB MASM_FILES ${CMAKE_SOURCE_DIR}/src/lammp/masm/*.asm)
        # 不需要转换为nasm，直接使用原始asm文件
    endif()
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
    
    if(CMAKE_C_COMPILER_ID MATCHES "GNU")
        target_compile_options(${target} PRIVATE
            $<$<COMPILE_LANGUAGE:C>:
                -Wall -Wextra -Wpedantic -fPIC -m64
                $<$<CONFIG:Debug>:-O0 -g -DDEBUG>
                $<$<CONFIG:Release>:-O3 -ffast-math -DNDEBUG>
                $<$<CONFIG:MinSizeRel>:-Os -DNDEBUG>
                $<$<CONFIG:RelWithDebInfo>:-O2 -g -DNDEBUG>
            >
            $<$<COMPILE_LANGUAGE:CXX>:
                -Wall -Wextra -Wpedantic -fPIC -m64
                $<$<CONFIG:Debug>:-O0 -g -DDEBUG>
                $<$<CONFIG:Release>:-O3 -ffast-math -DNDEBUG>
                $<$<CONFIG:MinSizeRel>:-Os -DNDEBUG>
                $<$<CONFIG:RelWithDebInfo>:-O2 -g -DNDEBUG>
            >
            # MinGW下MASM的编译选项
            $<$<COMPILE_LANGUAGE:ASM_MASM>:>
        )
        target_link_options(${target} PRIVATE
            -m64 -Wl,--enable-auto-import -Wl,--no-undefined
        )
    elseif(CMAKE_C_COMPILER_ID MATCHES "MSVC")
        target_compile_options(${target} PRIVATE
            $<$<COMPILE_LANGUAGE:C>:
                /MP /W3 /fp:fast /DWIN64 /D_CRT_SECURE_NO_WARNINGS
                $<$<CONFIG:Debug>:/Od /Zi /DDEBUG /MDd>
                $<$<CONFIG:Release>:/Ox /DNDEBUG /MD>
                $<$<CONFIG:MinSizeRel>:/Os /DNDEBUG /MD>
                $<$<CONFIG:RelWithDebInfo>:/O2 /Zi /DNDEBUG /MD>
            >
            $<$<COMPILE_LANGUAGE:CXX>:
                /MP /W3 /fp:fast /DWIN64
                $<$<CONFIG:Debug>:/Od /Zi /DDEBUG /MDd>
                $<$<CONFIG:Release>:/Ox /DNDEBUG /MD>
                $<$<CONFIG:MinSizeRel>:/Os /DNDEBUG /MD>
                $<$<CONFIG:RelWithDebInfo>:/O2 /Zi /DNDEBUG /MD>
            >
            $<$<COMPILE_LANGUAGE:ASM_MASM>:/nologo /DWIN64>
        )
        target_link_options(${target} PRIVATE
            /MACHINE:X64 /DYNAMICBASE /NXCOMPAT /SUBSYSTEM:CONSOLE
        )
    endif()
endfunction()

# ===================== 输出目录配置（核心修复：统一MSVC的DLL/LIB输出） =====================
set(OUTPUT_ROOT ${CMAKE_SOURCE_DIR}/dist/lammp)

# 对MSVC多配置构建的适配（最关键：分别设置RUNTIME/DLL和ARCHIVE/LIB的输出目录）
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

# ===================== 头文件路径 =====================
include_directories(${CMAKE_SOURCE_DIR}/include)

# ===================== 构建子模块 =====================
add_subdirectory(src/lammp)

if(EXISTS ${CMAKE_SOURCE_DIR}/main.cpp)
    add_executable(LammpMain ${CMAKE_SOURCE_DIR}/main.cpp)
    target_link_libraries(LammpMain PRIVATE LammpCore)
    set_common_compiler_options(LammpMain)
endif()

add_subdirectory(test/lammp)
add_subdirectory(example/lammp)
add_subdirectory(benchmark/lammp)