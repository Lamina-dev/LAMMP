cmake_minimum_required(VERSION 3.15)
project(lammp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MINGW)
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
    set(CMAKE_SHARED_LIBRARY_SUFFIX ".dll")
    add_compile_options(-Wall -Wextra -Wpedantic -O2 -fPIC -m64)
    add_link_options(-Wl,--enable-auto-import -Wl,--no-undefined)
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/include  # 已包含 include/，无需修改
    ${PROJECT_SOURCE_DIR}/src/lammp
)

# 修正1：仅收集核心库源码（排除 test）
file(GLOB_RECURSE LAMMP_SOURCES
    ${PROJECT_SOURCE_DIR}/src/lammp/*.cpp
    ${PROJECT_SOURCE_DIR}/src/lammp/*.cc
)

# 新增1：收集测试源码（src/test/ 下）
file(GLOB_RECURSE TEST_SOURCES
    ${PROJECT_SOURCE_DIR}/src/test/*.cpp
    ${PROJECT_SOURCE_DIR}/src/test/*.cc
)

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

# -------------------------- 编译可执行文件 --------------------------
# 修正2：添加 TEST_SOURCES（包含 src/test/Test.cpp）
add_executable(test_exec main.cpp ${TEST_SOURCES})

target_link_libraries(test_exec PRIVATE ${PROJECT_NAME})

# optional：调试模式
# target_compile_options(test_exec PRIVATE $<$<CONFIG:Debug>:-O0 -g>)
# -------------------------------------------------------------------