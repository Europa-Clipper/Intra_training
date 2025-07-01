# 收集 a、a1、a2 的源文件
project(httpLine)

# set(upload_SOURCES 
#     #src/a.cpp
#     Reader/src/PipeReader.cpp
#     UploadMgr/uploadMgr.cpp
# )

if(NOT CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/build" CACHE PATH "Installation prefix" FORCE)
endif()

add_executable(server server.cpp)
add_executable(client client.cpp)

# 添加 a 模块的动态库
#add_library(upload SHARED ${upload_SOURCES})

# 设置 a 模块的头文件搜索路径
target_include_directories(upload PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/../upload/Reader/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../upload/UploadMgr
)  

target_link_libraries(server utility upload pthread)
target_link_libraries(client utility upload pthread)
# 查找 pthread 库
find_package(Threads REQUIRED)

# 链接 pthread 库到可执行文件
target_link_libraries(server Threads::Threads)

# 安装可执行文件到 bin 目录
install(TARGETS server client DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

# set(INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin")
# file(MAKE_DIRECTORY ${INSTALL_BIN_DIR})
# install(TARGETS server client RUNTIME DESTINATION ${INSTALL_BIN_DIR})

# set(INSTALL_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../build/bin")
# install(TARGETS server DESTINATION ${INSTALL_BIN_DIR})
# install(TARGETS client DESTINATION ${INSTALL_BIN_DIR})