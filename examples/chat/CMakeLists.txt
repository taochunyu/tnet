add_executable(ChatServer chatServer.cc)
target_link_libraries(ChatServer tnet_net)

add_executable(ChatClient chatClient.cc)
target_link_libraries(ChatClient tnet_net)
