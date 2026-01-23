cmake_minimum_required(VERSION 3.10)
project(Lammp LANGUAGES CXX C ASM_MASM)

# 设置默认构建类型为 Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Default build type: Release" FORCE)
endif()

# 限制合法的构建类型
set(VALID_BUILD_TYPES Debug Release RelWithDebInfo MinSizeRel)
if(NOT CMAKE_BUILD_TYPE IN_LIST VALID_BUILD_TYPES)
    message(FATAL_ERROR "Invalid CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}, valid types are: ${VALID_BUILD_TYPES}")
endif()

# ===================== MASM 汇编器链接 =====================
if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 由于我的环境路径中没有ml64.exe，所以指定了ml64.exe路径
    set(ML64_PATH "D:/vs2022/IDE/VC/Tools/MSVC/14.44.35207/bin/Hostx64/x64/ml64.exe")
    message("please use your own ml64.exe path!")
    if(NOT EXISTS ${ML64_PATH})
        message(FATAL_ERROR "ml64.exe not found at: ${ML64_PATH}, please check the path!")
    endif()
    set(CMAKE_ASM_MASM_COMPILER ${ML64_PATH})

    set(CMAKE_ASM_MASM_FLAGS "/c /nologo /DWIN64")

    set(CMAKE_ASM_MASM_FLAGS_INIT "${CMAKE_ASM_MASM_FLAGS}")
    set(CMAKE_ASM_MASM_COMPILE_OBJECT "<CMAKE_ASM_MASM_COMPILER> <FLAGS> /Fo<OBJECT> <SOURCE>")
endif()

# ===================== C/C++选项 =====================
function(set_compiler_options target)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        # GCC/MinGW 选项
        target_compile_options(${target} PRIVATE
            $<$<COMPILE_LANGUAGE:C,CXX>:-Wall>
            $<$<COMPILE_LANGUAGE:C,CXX>:-Wextra>
            $<$<COMPILE_LANGUAGE:C,CXX>:-Wpedantic>
            $<$<COMPILE_LANGUAGE:C,CXX>:-W4> 
            $<$<COMPILE_LANGUAGE:C,CXX>:-fPIC>
            $<$<COMPILE_LANGUAGE:C,CXX>:-m64>
            $<$<AND:$<COMPILE_LANGUAGE:C,CXX>,$<CONFIG:Debug>>:-O0 -g -DDEBUG>
            $<$<AND:$<COMPILE_LANGUAGE:C,CXX>,$<CONFIG:Release>>:-O3 -DNDEBUG>
            $<$<AND:$<COMPILE_LANGUAGE:C,CXX>,$<CONFIG:MinSizeRel>>:-Os -DNDEBUG>
            $<$<AND:$<COMPILE_LANGUAGE:C,CXX>,$<CONFIG:RelWithDebInfo>>:-O2 -g -DNDEBUG>
        )
        target_link_options(${target} PRIVATE
            -Wl,--enable-auto-import -Wl,--no-undefined
        )
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        # MSVC 选项
        target_compile_options(${target} PRIVATE
            $<$<COMPILE_LANGUAGE:C,CXX>:/MP>
            $<$<COMPILE_LANGUAGE:C,CXX>:/fp:fast>
            $<$<COMPILE_LANGUAGE:C,CXX>:/DWIN64>
            $<$<AND:$<COMPILE_LANGUAGE:C,CXX>,$<CONFIG:Debug>>:/Od /Zi /DDEBUG>
            $<$<AND:$<COMPILE_LANGUAGE:C,CXX>,$<CONFIG:Release>>:/O2 /DNDEBUG>
            $<$<AND:$<COMPILE_LANGUAGE:C,CXX>,$<CONFIG:MinSizeRel>>:/Os /DNDEBUG>
            $<$<AND:$<COMPILE_LANGUAGE:C,CXX>,$<CONFIG:RelWithDebInfo>>:/O2 /Zi /DNDEBUG>
        )
        target_link_options(${target} PRIVATE
            /MACHINE:X64 /DYNAMICBASE /NXCOMPAT
        )
    endif()
endfunction()

# ===================== 基础配置 =====================
# 设置C++17标准 C11标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 配置编译产物输出目录
set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/dist/lammp)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR}/lib)


# 引入子模块
add_subdirectory(src/lammp)

# 主程序（若有）
if(EXISTS ${CMAKE_SOURCE_DIR}/main.cpp)
    add_executable(LammpMain ${CMAKE_SOURCE_DIR}/main.cpp)
    target_link_libraries(LammpMain PRIVATE LammpCore)
    # 给主程序设置C/C++选项
    set_compiler_options(LammpMain)
endif()

add_subdirectory(benchmark/lammp)
add_subdirectory(example/lammp)
add_subdirectory(test/lammp)