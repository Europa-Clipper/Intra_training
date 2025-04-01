# 收集 a、a1、a2 的源文件
project(upload)

set(upload_SOURCES 
    #src/a.cpp
    Reader/src/PipeReader.cpp
    #a2/src/a2.cpp
)

# 添加 a 模块的动态库
add_library(upload SHARED ${upload_SOURCES})

# 设置 a 模块的头文件搜索路径
target_include_directories(upload PUBLIC 
    #${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/Reader/include
    #${CMAKE_CURRENT_SOURCE_DIR}/a2/include
)  

target_link_libraries(upload utility)