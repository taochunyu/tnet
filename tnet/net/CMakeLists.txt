set(net_SRCS
  Acceptor.cc
  Buffer.cc
  Channel.cc
  EventLoop.cc
  EventLoopThread.cc
  EventLoopThreadPool.cc
  InetAddress.cc
  Poller.cc
  poller/DefaultPoller.cc
  poller/PollPoller.cc
  Socket.cc
  SocketsOps.cc
  TcpConnection.cc
  TcpServer.cc
  Timer.cc
  TimerQueue.cc
)

add_library(tnet_net ${net_SRCS})
target_link_libraries(tnet_net tnet_base)
