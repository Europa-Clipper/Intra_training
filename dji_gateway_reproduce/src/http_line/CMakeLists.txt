# 收集 a、a1、a2 的源文件
project(httpLine)

# set(upload_SOURCES 
#     #src/a.cpp
#     Reader/src/PipeReader.cpp
#     UploadMgr/uploadMgr.cpp
# )

add_executable(server server.cpp)
add_executable(client client.cpp)

# 添加 a 模块的动态库
#add_library(upload SHARED ${upload_SOURCES})

# 设置 a 模块的头文件搜索路径
# target_include_directories(upload PUBLIC 
#     #${CMAKE_CURRENT_SOURCE_DIR}/include
#     ${CMAKE_CURRENT_SOURCE_DIR}/Reader/include
#     ${CMAKE_CURRENT_SOURCE_DIR}/UploadMgr
# )  

target_link_libraries(server utility)
target_link_libraries(client utility)
# 查找 pthread 库
find_package(Threads REQUIRED)

# 链接 pthread 库到可执行文件
target_link_libraries(server Threads::Threads)