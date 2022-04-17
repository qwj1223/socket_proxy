cmake_minimum_required(VERSION 3.10.0)
project(mdb_proxy)

set (CMAKE_CXX_FLAGS "--std=c++11")

add_executable(mdb_proxy ${PROJECT_SOURCE_DIR}/source/main.cpp)
target_link_libraries(mdb_proxy socket_proxy)
target_include_directories(mdb_proxy PUBLIC ${PROJECT_SOURCE_DIR}/include/ /home/wilson/socket/socket_proxy/include/)
