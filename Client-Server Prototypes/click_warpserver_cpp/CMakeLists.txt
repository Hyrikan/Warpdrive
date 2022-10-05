cmake_minimum_required(VERSION 3.16)
project(click_warpserver_cpp)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/)

link_libraries(warp_server ucp liblz4-lib.a libclickhouse-cpp-lib.so libabsl-lib.a libcityhash-lib.a)
link_directories(/usr/lib/warpdrive/ /usr/lib/ucx/ ${CMAKE_SOURCE_DIR}/)

set(SOURCES main.cpp)
add_executable(click_warpserver_cpp ${SOURCES})
