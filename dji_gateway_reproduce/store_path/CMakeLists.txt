cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME dji-gateway)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

project(${PROJECT_NAME})

#set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
execute_process(COMMAND ${CMAKE_CXX_COMPILER} --version OUTPUT_VARIABLE CXX_COMPILER_VERSION)
message(STATUS "C++ Compiler version: ${CXX_COMPILER_VERSION}")

add_subdirectory(src/utility)
add_subdirectory(src/upload)
add_subdirectory(src/http_line)
#add_subdirectory(src/resource_monitor)

add_executable(${PROJECT_NAME} 
    src/main.cpp
)

target_link_libraries(${PROJECT_NAME} utility upload httpLine pthread)

target_include_directories(${PROJECT_NAME} PUBLIC 
    src/upload/include
    src/utility/include
    #http_line
)    