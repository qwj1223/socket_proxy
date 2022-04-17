cmake_minimum_required(VERSION 3.10.0)
project(socket_proxy)

set (CMAKE_CXX_FLAGS "--std=c++11")

add_library(socket_proxy STATIC ${PROJECT_SOURCE_DIR}/src/mdb_proxy_sockapi.cpp)
#target_link_libraries(socket_proxy pthread)
target_include_directories(socket_proxy PUBLIC ${PROJECT_SOURCE_DIR}/include/)

#add_executable(socket_proxy )
#target_link_libraries(test_thread wzq_thread)
