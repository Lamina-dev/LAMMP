# 1. 递归收集基准测试的源代码（include为私有头，src为源码，main.cpp为主程序）
file(GLOB_RECURSE BENCH_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

# 2. 包含基准测试私有头文件目录（仅当前模块使用，不对外暴露）
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 3. 生成基准测试可执行文件（名称：LammpBenchmark，可自定义）
add_executable(LammpBenchmark ${BENCH_SRCS})

# 4. 链接核心动态库LammpCore（必须链接，否则无法调用项目核心功能）
# PRIVATE表示该库仅当前可执行文件使用，不传递给依赖当前目标的其他目标
target_link_libraries(LammpBenchmark PRIVATE LammpCore)

# 5. （可选）设置可执行文件输出目录（若需单独存放，可覆盖根目录配置）
# set_target_properties(LammpBenchmark PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/dist/benchmark_bin
# )