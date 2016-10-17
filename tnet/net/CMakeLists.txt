set(net_SRCS
  Channel.cc
  EventLoop.cc
  EventLoopThread.cc
  InetAddress.cc
  Poller.cc
  poller/DefaultPoller.cc
  poller/PollPoller.cc
  Socket.cc
  SocketsOps.cc
  Timer.cc
  TimerQueue.cc
)

add_library(tnet_net ${net_SRCS})
target_link_libraries(tnet_net tnet_base)
set_target_properties(tnet_net PROPERTIES COMPILE_FLAGS "-std=c++11")
