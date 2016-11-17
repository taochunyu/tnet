cmake_minimum_required(VERSION 3.4)

project(tnet C CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -Wall -g -std=c++14")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -finline-limit=1000 -DNDEBUG")

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(tnet)

if(NOT CMAKE_BUILD_NO_TEST)
  enable_testing()
  add_subdirectory(test)
endif(NOT CMAKE_BUILD_NO_TEST)

add_subdirectory(examples)
