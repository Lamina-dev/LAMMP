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