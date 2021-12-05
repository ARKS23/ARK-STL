cmake_minimum_required(VERSION 3.17)
project(My_STL)

set(CMAKE_CXX_STANDARD 14)
set(INC_DIR /Users/chenshen/Downloads/Markdown/Linux/Libevent/libevent-2.0.22-stable/include)
set(LINK_DIR /Users/chenshen/Downloads/Markdown/Linux/Libevent/libevent-2.0.22-stable/.libs)

AUX_SOURCE_DIRECTORY(cmake-build-debug/MySTL/ stl)
include_directories(${INC_DIR})
link_directories(${LINK_DIR})
link_libraries(event)

add_executable(My_STL main.cpp ${stl})
target_link_libraries(My_STL event)
