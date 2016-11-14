# EchoServer
add_executable(EchoServer EchoServer.cc)
target_link_libraries(EchoServer tnet_net)
# add_test(EchoServerTest EchoServer)

# EchoClient
add_executable(EchoClient EchoClient.cc)
target_link_libraries(EchoClient tnet_net)
# add_test(EchoClientTest EchoClient)
