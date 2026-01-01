# 1. 递归收集当前目录（src/lammp）下的所有源代码
# ${CMAKE_CURRENT_SOURCE_DIR} 指向当前CMake文件所在目录（src/lammp）
file(GLOB_RECURSE LAMMP_CORE_SRCS 
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${PROJECT_SOURCE_DIR}/src/lammp/*.cc
)

# 2. 生成动态库（SHARED指定动态库，库名称LammpCore）
add_library(LammpCore SHARED ${LAMMP_CORE_SRCS})

# 3. 设置动态库属性（统一名称、版本号）
set_target_properties(LammpCore PROPERTIES
    OUTPUT_NAME "LammpCore"
    VERSION 1.0.0
    SOVERSION 1
)

# 4. 【可选】若需仅对核心库生效编译/链接选项，可在此配置（替代全局配置）
# 局部编译选项（仅LammpCore生效）
target_compile_options(LammpCore PRIVATE -Wall -Wextra -Wpedantic -O3 -fPIC -m64)

# 5. 【可选】局部链接选项（仅LammpCore生效）
target_link_options(LammpCore PRIVATE -Wl,--enable-auto-import -Wl,--no-undefined)