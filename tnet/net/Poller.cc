#include <tnet/net/Poller.h>
#include <tnet/net/Channel.h>

using namespace tnet;
using namespace tnet::net;

Poller::Poller(EventLoop* loop)
  : _ownerloop(loop) {}

Poller::~Poller() {}

bool Poller::hasChannel(Channel* channel) const {
  assertInLoopThread();
  ChannelMap::const_iterator it = _channels.find(channel -> fd());
  return it != _channels.end() && it -> second == channel;
}
