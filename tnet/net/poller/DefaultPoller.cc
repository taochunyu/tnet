#include <tnet/net/poller/PollPoller.h>
#include <tnet/net/Poller.h>
#include <tnet/net/EventLoop.h>

using namespace tnet::net;

Poller* Poller::newDefaultPoller(EventLoop* loop) {
  return new PollPoller(loop);
}
