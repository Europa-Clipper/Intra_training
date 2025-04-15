# 设置 CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
set(PROJECT_NAME dji-gateway)

# 设置项目名
project(${PROJECT_NAME})

# 设置 C++ 标准
#set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

# 添加子目录
add_subdirectory(src/utility)
add_subdirectory(src/upload)
add_subdirectory(src/http_line)

# 生成可执行文件
add_executable(${PROJECT_NAME} src/main.cpp)

# 链接 a、b、c 库
target_link_libraries(${PROJECT_NAME} utility upload)

# 设置可执行文件的头文件搜索路径
target_include_directories(${PROJECT_NAME} PUBLIC 
    upload/include
    utility/include
)    