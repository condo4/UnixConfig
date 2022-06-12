cmake_minimum_required(VERSION 3.2)
project(UnixConfig)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES config.cpp config.h)

add_library(UnixConfig STATIC ${SOURCE_FILES})
