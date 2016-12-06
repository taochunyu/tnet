set(server_SRC
  server.cc
  FileModel.cc
  FileServer.cc
  MessageServer.cc
  Task.cc
)

set(client_SRC
  client.cc
  FileModel.cc
  FileClient.cc
  MessageClient.cc
  Task.cc
)

add_executable(server ${server_SRC})
target_link_libraries(server tnet_net)

add_executable(client ${client_SRC})
target_link_libraries(client tnet_net)
