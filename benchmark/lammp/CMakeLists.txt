 # LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 # This file is part of lammp, under the GNU LGPL v2 license.
 # See LICENSE in the project root for the full license text.

# 收集基准测试源代码
file(GLOB_RECURSE BENCH_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

# 生成基准测试可执行文件
add_executable(LammpBenchmark ${BENCH_SRCS})

# 头文件包含
target_include_directories(LammpBenchmark
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include
)

# 链接核心动态库
target_link_libraries(LammpBenchmark PRIVATE LammpCore)

# 设置编译器选项
lammp_target_setup(LammpBenchmark)

# 设置输出属性
set_target_properties(LammpBenchmark PROPERTIES
    OUTPUT_NAME "LammpBenchmark"
    PDB_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
