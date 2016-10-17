cmake_minimum_required(VERSION 3.4)

project(tnet C CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -Wall -g -std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -finline-limit=1000 -DNDEBUG")

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(tnet)
add_subdirectory(test)
# add_subdirectory(example example)
