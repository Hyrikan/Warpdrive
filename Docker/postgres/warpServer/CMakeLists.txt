cmake_minimum_required(VERSION 3.16)
project(pg_standalone_warpServer C)

set(CMAKE_C_STANDARD 99)


link_libraries(warp_server ucp pq arrow-glib glib-2.0 gobject-2.0)
link_directories(/usr/lib/warpdrive/ /usr/lib/ucx/)
include_directories(/usr/include/postgresql/ /usr/include/glib-2.0/ /usr/lib/x86_64-linux-gnu/glib-2.0/include/)


add_executable(pg_standalone_warpServer main.c)
#add_executable(testarray examplearrowarray.c)
#add_executable(teststream examplearrowstream.c)
#add_executable(test test.c)
