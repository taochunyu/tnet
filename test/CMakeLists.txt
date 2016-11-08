# GTest headers and libraries
find_package(GTest REQUIRED)
find_library(GTEST_MAIN NAMES gtest_main REQUIRED)
message(STATUS "Found GTest headers in " ${GTEST_INCLUDE_DIRS})
message(STATUS "Found GTest libraries in " ${GTEST_LIBRARIES})
message(STATUS "Found GTest library gtest_main in " ${GTEST_MAIN})
include_directories(${GTEST_INCLUDES})


add_subdirectory(tnet)
add_subdirectory(byExamples)
