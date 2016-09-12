message(STATUS "This is source dir " ${TNET_BASE_SRCS})
add_executable(poll_test poll_test.cc)
target_link_libraries(tnet_base)
