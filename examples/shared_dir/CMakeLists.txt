set(server_SRC
  Server.cc
  FileModel.cc
  MessageServer.cc
)

set(client_SRC
  Client.cc
  FileModel.cc
  MessageClient.cc
)

add_executable(Server ${server_SRC})
target_link_libraries(Server tnet_net)

add_executable(Client ${client_SRC})
target_link_libraries(Client tnet_net)
