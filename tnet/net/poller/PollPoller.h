#ifndef TNET_NET_POLLPOLLER_H
#define TNET_NET_POLLPOLLER_H

#include <tnet/net/Poller.h>
#include <vector>

struct pollfd;

namespace tnet {
namespace net {

class PollPoller : public Poller {
 public:
  PollPoller(EventLoop* loop);
  virtual ~PollPoller();

  virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels);
  virtual void updateChannel(Channel* channel);
  virtual void removeChannel(Channel* channel);

 private:
  using PollFdList = std::vector<struct pollfd>;
  void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
  PollFdList _pollfds;
};

}
}  // namespace tnet

#endif  // TNET_NET_POLLPOLLER_H
