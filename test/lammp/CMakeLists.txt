# 1. 递归收集测试源代码（include私有头，src源码，main.cpp测试主程序）
file(GLOB_RECURSE TEST_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

# 2. 包含测试私有头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 3. 生成测试可执行文件（名称：LammpTest，可自定义）
add_executable(LammpTest ${TEST_SRCS})

# 4. 链接核心动态库LammpCore
target_link_libraries(LammpTest PRIVATE LammpCore)
