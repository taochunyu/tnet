#ifndef TNET_NET_POLLER_H
#define TNET_NET_POLLER_H

#include <tnet/base/nocopyable.h>
#include <tnet/base/Timestamp.h>
#include <map>
#include <vector>

namespace tnet {
namespace net {

class Channel;
class EventLoop;
// base class for I/O Multiplexing
// this class doesn't own the Channel Objects
class Poller : tnet::nocopyable {
 public:
  using ChannelList = std::vector<Channel*>;
  Poller(EventLoop *loop);
  virtual ~Poller();

  // Polls the I/O events
  // Must be called in the loop thread
  virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;
  // Changes the interested I/O events;
  // Must be called in the loop thread;
  virtual void updateChannel(Channel* channel) = 0;
  // remove the channel, when it destructs;
  // Must be called in the loop thread;
  virtual void removeChannel(Channel* channel) = 0;
  virtual bool hasChannel(Channel* channel) const;

  void assertInLoopThread() const {
    _ownerloop -> assertInLoopThread();
  }
  static Poller* newDefaultPoller(EventLoop* loop);
 protected:
  using ChannelMap = std::map<int, Channel*>;
  ChannelMap _channels;
 private:
  EventLoop* _ownerloop;
};

}  // namespace net;
}  // namespace tnet

#endif  // TNET_NET_EVENT_LOOP_H
