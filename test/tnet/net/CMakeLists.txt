# TimerQueueTest
add_executable(TimerQueueTest TimerQueue_gtest.cc)
target_link_libraries(TimerQueueTest tnet_net ${GTEST_LIBRARIES} ${GTEST_MAIN})
add_test(TimerQueueTest TimerQueueTest)
