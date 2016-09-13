cmake_minimum_required(VERSION 3.4)

project(tnet C CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CXX_FLAGS
 -g
 -Wall
 -std=c++11
 )

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -finline-limit=1000 -DNDEBUG")
set(PROJECT_SOURCE_DIR /Users/taochunyu/worktable/tnet)

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(tnet)
add_subdirectory(whenDeveloping)
# add_subdirectory(example example)
