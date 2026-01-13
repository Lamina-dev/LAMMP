file(GLOB_RECURSE TEST_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(LammpTest ${TEST_SRCS})

# 链接动态库LammpCore
target_link_libraries(LammpTest PRIVATE LammpCore)
