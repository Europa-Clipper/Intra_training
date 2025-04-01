# 添加 b 模块的动态库
add_library(utility SHARED 
    src/dlog.cpp
    src/IFStream.cpp
)

# 设置 b 模块的头文件搜索路径
target_include_directories(utility PUBLIC 
    #${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/utility/include
    #${CMAKE_SOURCE_DIR}/../upload/Reader/include
    #${CMAKE_SOURCE_DIR}/a/a2/include
)

# 链接 a 库
#target_link_libraries(b a) 