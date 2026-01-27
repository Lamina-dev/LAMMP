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
set_common_compiler_options(LammpBenchmark)

# 设置输出属性
set_target_properties(LammpBenchmark PROPERTIES
    OUTPUT_NAME "LammpBenchmark"
    PDB_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# 设置可执行文件输出目录（若需单独存放，可覆盖根目录配置）
# set_target_properties(LammpBenchmark PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/dist/benchmark_bin
# )