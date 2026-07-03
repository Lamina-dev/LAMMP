#
#  Copyright (C) 2026 HJimmyK(Jericho Knox)
#
#  This file is part of LAMMP.
#
#  LAMMP is free software: you can redistribute it and/or modify it under
#  the terms of the GNU Lesser General Public License (LGPL) as published
#   by the Free Software Foundation; either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed WITHOUT ANY WARRANTY. 
#
#  See <https://www.gnu.org/licenses/>.

# 收集测试源代码
file(GLOB_RECURSE TEST_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

# 生成测试可执行文件
add_executable(LammpTest ${TEST_SRCS})

# 头文件包含
target_include_directories(LammpTest
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include
)

# 链接核心动态库
target_link_libraries(LammpTest PRIVATE LammpCore)

# 设置编译器选项
lammp_target_setup(LammpTest)

# 设置输出名称和属性
set_target_properties(LammpTest PROPERTIES
    OUTPUT_NAME "LammpTest"
    # Windows PDB文件输出
    PDB_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)