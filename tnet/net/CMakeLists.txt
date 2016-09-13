set(net_SRCS
  Channel.cc
  EventLoop.cc
  Poller.cc
  poller/PollPoller.cc
)

add_library(tnet_net ${net_SRCS})
target_link_libraries(tnet_net tnet_base)
