cmake_minimum_required(VERSION 3.10.0)
project(mdb_proxy)

set (CMAKE_CXX_FLAGS "--std=c++11")

add_subdirectory(socket_proxy)

include_directories(${PROJECT_SOURCE_DIR}/socket_proxy/include)
link_directories(${PROJECT_SOURCE_DIR}/socket_proxy/build)

add_executable(mdb_proxy ${PROJECT_SOURCE_DIR}/source/main.cpp ${PROJECT_SOURCE_DIR}/source/mdb_proxy.cpp)
target_link_libraries(mdb_proxy socket_proxy pthread)
target_include_directories(mdb_proxy PUBLIC ${PROJECT_SOURCE_DIR}/include)
