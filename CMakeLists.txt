# cmake_minimum_required(VERSION 3.15)
# project(lammp LANGUAGES C CXX)

# set(CMAKE_C_STANDARD 11)
# set(CMAKE_C_STANDARD_REQUIRED ON)
# set(CMAKE_C_EXTENSIONS ON)

# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)

# # 1. Release模式
# set(CMAKE_BUILD_TYPE Release CACHE STRING "强制Release模式（性能最优）" FORCE)

# # 2. 编译选项
# if(MSVC)
#     # MSVC：(/O2) + (/Gy) + (/GF)
#     set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2 /Gy /GF /GL")
#     set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Gy /GF /GL")
#     # 禁用调试信息（MSVC）
#     set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi-")
#     set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi-")
# else()
#     # GCC/Clang/MinGW：-O3（性能最优），拆分函数/数据段
#     set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -fPIC -ffunction-sections -fdata-sections")
#     set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -fPIC -ffunction-sections -fdata-sections")
#     # 隐藏非导出符号
#     set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fvisibility=hidden -fvisibility-inlines-hidden")
#     set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fvisibility=hidden")
#     # 禁用调试信息
#     set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -g0")
#     set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g0")
# endif()

# # 3. 链接选项
# if(MSVC)
#     # MSVC：剔除未引用代码(/OPT:REF) + 合并重复函数(/OPT:ICF) + 链接时优化(/LTCG)
#     set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /OPT:REF /OPT:ICF /LTCG")
#     set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /OPT:REF /OPT:ICF /LTCG")
# elseif(MINGW)
#     # MinGW 专属链接选项（移除--icf=safe，保留其他有效优化）
#     set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -Wl,--gc-sections -flto -s")
#     set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--gc-sections -flto -s")
# else()
#     # Linux/macOS (GCC/Clang)：保留所有兼容优化
#     set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -Wl,--gc-sections -Wl,--icf=safe -flto -s")
#     set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--gc-sections -Wl,--icf=safe -flto -s")
# endif()

# if(MINGW)
#     set(CMAKE_SHARED_LIBRARY_PREFIX "")
#     set(CMAKE_SHARED_LIBRARY_SUFFIX ".dll")
#     add_compile_options(-Wall -Wextra -Wpedantic -m64)
#     add_link_options(-Wl,--enable-auto-import -Wl,--no-undefined)
# endif()

# # 源码收集（保持原有逻辑）
# file(GLOB_RECURSE LAMMP_SOURCES
#     ${PROJECT_SOURCE_DIR}/src/lammp/*.cpp
#     ${PROJECT_SOURCE_DIR}/src/lammp/*.cc
# )

# file(GLOB_RECURSE TEST_SOURCES
#     ${PROJECT_SOURCE_DIR}/src/test/*.cpp
#     ${PROJECT_SOURCE_DIR}/src/test/*.cc
# )

# # 编译库和可执行文件
# add_library(${PROJECT_NAME} SHARED ${LAMMP_SOURCES})

# install(
#     TARGETS ${PROJECT_NAME}
#     RUNTIME DESTINATION bin
#     LIBRARY DESTINATION lib
#     ARCHIVE DESTINATION lib
# )

# install(
#     FILES ${PROJECT_SOURCE_DIR}/include/lammp/lammp.hpp
#     DESTINATION include/lammp
# )

# add_executable(test_exec main.cpp ${TEST_SOURCES})
# target_link_libraries(test_exec PRIVATE ${PROJECT_NAME})

cmake_minimum_required(VERSION 3.15)
project(lammp LANGUAGES C CXX)

# 跨平台标准配置
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# MinGW 平台
if(MINGW)
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
    set(CMAKE_SHARED_LIBRARY_SUFFIX ".dll")
    # 编译选项
    add_compile_options(-Wall -Wextra -Wpedantic -O3 -fPIC -m64)
    add_link_options(-Wl,--enable-auto-import -Wl,--no-undefined)
endif()

# 源码收集
file(GLOB_RECURSE LAMMP_SOURCES
    ${PROJECT_SOURCE_DIR}/src/lammp/*.cpp
    ${PROJECT_SOURCE_DIR}/src/lammp/*.cc
)

file(GLOB_RECURSE TEST_SOURCES
    ${PROJECT_SOURCE_DIR}/src/test/*.cpp
    ${PROJECT_SOURCE_DIR}/src/test/*.cc
)

# 编译库和可执行文件
add_library(${PROJECT_NAME} SHARED ${LAMMP_SOURCES})

install(
    TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(
    FILES ${PROJECT_SOURCE_DIR}/include/lammp/lammp.hpp
    DESTINATION include/lammp
)

add_executable(test_exec main.cpp ${TEST_SOURCES})
target_link_libraries(test_exec PRIVATE ${PROJECT_NAME})