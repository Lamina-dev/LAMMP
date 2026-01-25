# 收集所有示例文件
file(GLOB EXAMPLE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/example*.cpp)

# 遍历生成示例可执行文件
foreach(EXAMPLE_FILE ${EXAMPLE_FILES})
    # 获取文件名（不含路径和扩展名）
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_FILE} NAME_WE)
    # 生成可执行文件名称（如Lammpexample1）
    set(TARGET_NAME "lammp_${EXAMPLE_NAME}")
    
    # 添加可执行文件
    add_executable(${TARGET_NAME} ${EXAMPLE_FILE})
    
    # 头文件包含
    target_include_directories(${TARGET_NAME}
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
    )
    
    # 链接核心动态库
    target_link_libraries(${TARGET_NAME} PRIVATE LammpCore)
    
    # 设置编译器选项
    set_common_compiler_options(${TARGET_NAME})
    
    # 设置输出属性
    set_target_properties(${TARGET_NAME} PROPERTIES
        OUTPUT_NAME ${TARGET_NAME}
        PDB_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
endforeach()