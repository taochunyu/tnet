#include <tnet/net/PollPoller.h>
#include <tnet/base/Logging.h>
#include <tnet/net/Channel.h>
#include <assert.h>
#include <errno.h>
#include <poll.h>

using namespace tnet;
using namespace tnet::net;

PollPoller::PollPoller(EventLoop* loop)
  : Poller(loop)
{}

PollPoller::PollPoller() {}

Timestamp PollPoller::poll(int timeoutMs, ChannelList* activeChannels) {
  // _pollfds shouldn't change
  int numEvents = ::poll(&*_pollfds.begin(), _pollfds.size(), timeoutMs);
  int savedError = errno;
  Timestamp now(Timestamp::now());
  if (numEvents > 0) {
    LOG_TRACE << numEvents << " events happened";
    
  }
}
