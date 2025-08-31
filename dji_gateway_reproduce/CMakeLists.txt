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

include_directories(/usr/local/include)
link_directories(/usr/local/lib)
set(LWS_LIBRARY websockets)

add_subdirectory(src/utility)
add_subdirectory(src/upload)
add_subdirectory(src/http_line)
#add_subdirectory(src/resource_monitor)

# include_directories(
#     ${PROJECT_SOURCE_DIR}/3rdParty/json-3.12.0/include
# )
set(LWS_INCLUDE_DIRS /usr/local/include)
set(LWS_LIBRARIES /usr/local/lib/libwebsockets.so)
find_package(libwebsockets REQUIRED)


add_executable(${PROJECT_NAME} 
    src/main.cpp
    #src/chatServer/src/cli.cpp
    #src/chatServer/src/serv.cpp
    src/resource_monitor/src/ws_server.cpp
    src/resource_monitor/src/ws_client.cpp
)


target_link_libraries(${PROJECT_NAME} 
                        utility
                        upload
                        httpLine
                        #${CMAKE_THREAD_LIBS_INIT}
                        pthread
                        # ${LWS_LIBRARIES} 
                        ixwebsocket
                        z
                        ssl
                        crypto
                        )

target_include_directories(${PROJECT_NAME} PUBLIC 
    src/upload/include
    src/utility/include
    ${LWS_INCLUDE_DIRS} 
    #http_line
    ${PROJECT_SOURCE_DIR}/3rdParty/json-3.12.0/include
    src/resource_monitor/include
)    
message(STATUS "dji-gateway CXX Flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "dji-gateway CXX Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "libwebsockets include dir: ${LWS_INCLUDE_DIRS}")
message(STATUS "libwebsockets libraries: ${LWS_LIBRARIES}")