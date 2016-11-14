# TimerQueueTest
add_executable(TimerQueueTest TimerQueue_gtest.cc)
target_link_libraries(TimerQueueTest tnet_net ${GTEST_LIBRARIES} ${GTEST_MAIN})
add_test(TimerQueueTest TimerQueueTest)

# InetAddressTest
add_executable(InetAddressTest InetAddress_gtest.cc)
target_link_libraries(InetAddressTest tnet_net ${GTEST_LIBRARIES} ${GTEST_MAIN})
add_test(InetAddressTest InetAddressTest)

# TcpClient
add_executable(TcpClient TcpClient_reg2.cc)
target_link_libraries(TcpClient tnet_net)
# add_test(TcpClientTest TcpClient)
