project(httpLine)

set(httpLine_SOURCES 
    #src/a.cpp
    server.cpp
)

add_library(httpLine SHARED ${httpLine_SOURCES})

# target_include_directories(httpLine PUBLIC 
#     # ${CMAKE_CURRENT_SOURCE_DIR}/../upload/Reader/include
#     # ${CMAKE_CURRENT_SOURCE_DIR}/../upload/UploadMgr
#     #${CMAKE_CURRENT_SOURCE_DIR}/../utility/include
# )  

target_include_directories(httpLine PUBLIC 
  ${CMAKE_CURRENT_SOURCE_DIR}
)

find_package(Threads REQUIRED)

target_link_libraries(httpLine PUBLIC utility)