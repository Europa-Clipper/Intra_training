# # 收集 a、a1、a2 的源文件
# project(resrcmnt)

# if(NOT CMAKE_INSTALL_PREFIX)
#     set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/build" CACHE PATH "Installation prefix" FORCE)
# endif()

# set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)
# set(CMAKE_COLOR_DIAGNOSTICS ON)
# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

# set(resrcmnt_SOURCES 
#     #src/a.cpp
#     #Reader/src/PipeReader.cpp
#     #UploadMgr/uploadMgr.cpp
#     src/cpu.cpp
# )



# # 添加 a 模块的动态库
# #add_library(resrcmnt ${resrcmnt_SOURCES})


# #add_executable(resrcmntor src/cpu.cpp)

# # 设置 a 模块的头文件搜索路径
# target_include_directories(resrcmnt PUBLIC 
#     ${CMAKE_CURRENT_SOURCE_DIR}/include
#     #${CMAKE_CURRENT_SOURCE_DIR}/Reader/include
#     #${CMAKE_CURRENT_SOURCE_DIR}/UploadMgr
# )  

# #target_link_libraries(upload utility)

# #install(TARGETS resrcmnt RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)