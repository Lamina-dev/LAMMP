file(GLOB_RECURSE BENCH_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(LammpBenchmark ${BENCH_SRCS})
target_link_libraries(LammpBenchmark PRIVATE LammpCore)

# 设置可执行文件输出目录（若需单独存放，可覆盖根目录配置）
# set_target_properties(LammpBenchmark PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/dist/benchmark_bin
# )