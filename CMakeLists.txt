cmake_minimum_required(VERSION 3.15)
project(lammp LANGUAGES C CXX)

# 跨平台标准配置
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# MinGW 平台特殊配置（修正语法错误）
if(MINGW)
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
    set(CMAKE_SHARED_LIBRARY_SUFFIX ".dll")
    # 编译选项：生成器表达式完整闭合（$<...:>）
    add_compile_options(-Wall -Wextra -Wpedantic -O0 -fPIC -m64)
    add_link_options(-Wl,--enable-auto-import -Wl,--no-undefined)
endif()

# 源码收集（保持原有逻辑）
file(GLOB_RECURSE LAMMP_SOURCES
    ${PROJECT_SOURCE_DIR}/src/lammp/*.cpp
    ${PROJECT_SOURCE_DIR}/src/lammp/*.cc
)

file(GLOB_RECURSE TEST_SOURCES
    ${PROJECT_SOURCE_DIR}/src/test/*.cpp
    ${PROJECT_SOURCE_DIR}/src/test/*.cc
)

# 编译库和可执行文件（保持原有逻辑）
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