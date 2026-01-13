# 示例1：生成独立可执行文件
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/example1.cpp)
    add_executable(LammpExample1 ${CMAKE_CURRENT_SOURCE_DIR}/example1.cpp)
    target_link_libraries(LammpExample1 PRIVATE LammpCore)
endif()

# 示例2：生成独立可执行文件
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/example2.cpp)
    add_executable(LammpExample2 ${CMAKE_CURRENT_SOURCE_DIR}/example2.cpp)
    target_link_libraries(LammpExample2 PRIVATE LammpCore)
endif()

# 示例3：生成独立可执行文件
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/example3.cpp)
    add_executable(LammpExample3 ${CMAKE_CURRENT_SOURCE_DIR}/example3.cpp)
    target_link_libraries(LammpExample3 PRIVATE LammpCore)
endif()

# 示例4：生成独立可执行文件
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/example4.cpp)
    add_executable(LammpExample4 ${CMAKE_CURRENT_SOURCE_DIR}/example4.cpp)
    target_link_libraries(LammpExample4 PRIVATE LammpCore)
endif()