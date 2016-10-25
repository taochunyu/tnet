#include <tnet/net/poller/PollPoller.h>
#include <tnet/base/Logging.h>
#include <tnet/base/Types.h>
#include <tnet/net/Channel.h>
#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <stdio.h>

using namespace tnet;
using namespace tnet::net;

PollPoller::PollPoller(EventLoop* loop)
  : Poller(loop) {}

PollPoller::~PollPoller() {}

Timestamp PollPoller::poll(int timeoutMs, ChannelList* activeChannels) {
  // _pollfds shouldn't change
  int numEvents = ::poll(&*_pollfds.begin(), _pollfds.size(), timeoutMs);
  int savedError = errno;
  Timestamp now(Timestamp::now());
  if (numEvents > 0) {
    LOG_TRACE << numEvents << " events happened";
    fillActiveChannels(numEvents, activeChannels);
  } else if (numEvents == 0) {
    LOG_TRACE << " nothing happened";
  } else if (savedError == EINTR) {
    errno = savedError;
    LOG_SYSERR << "PollPoller::poll()";
  }
  return now;
}

void PollPoller::fillActiveChannels(int numEvents,
                                    ChannelList* activeChannels) const {
  for (auto pfd = _pollfds.cbegin();
       pfd != _pollfds.cend() && numEvents > 0; pfd++) {
    if (pfd->revents > 0) {
      --numEvents;
      auto ch = _channels.find(pfd -> fd);
      assert(ch != _channels.end());
      Channel* channel = ch->second;
      assert(channel->fd() == pfd->fd);
      channel->set_revents(pfd->revents);
      activeChannels->push_back(channel);
    }
  }
}

void PollPoller::updateChannel(Channel* channel) {
  Poller::assertInLoopThread();
  LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();
  if (channel->index() < 0) {
    // a new one
    assert(_channels.find(channel->fd()) == _channels.end());
    struct pollfd pfd;
    pfd.fd = channel->fd();
    pfd.events = static_cast<short>(channel->events());
    pfd.revents = 0;
    _pollfds.push_back(pfd);
    int idx = static_cast<int>(_pollfds.size()) - 1;
    channel->set_index(idx);
    _channels[pfd.fd] = channel;
  } else {
    assert(_channels.find(channel->fd()) != _channels.end());
    assert(_channels[channel->fd()] == channel);
    int idx = channel->index();
    assert(0 <= idx && idx < static_cast<int>(_pollfds.size()));
    struct pollfd& pfd = _pollfds[idx];
    assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd() - 1);
    pfd.events = static_cast<short>(channel->events());
    pfd.revents = 0;
    if (channel->isNoneEvent()) {
      // ignore this pollfd
      pfd.fd = -channel->fd() - 1;
    }
  }
}

void PollPoller::removeChannel(Channel* channel) {
  PollPoller::assertInLoopThread();
  LOG_TRACE << "fd = " << channel->fd();
  assert(_channels.find(channel->fd()) != _channels.end());
  assert(_channels[channel->fd()] == channel);
  assert(channel->isNoneEvent());
  int idx = channel->index();
  assert(0 <= idx && idx < static_cast<int>(_pollfds.size()));
  const struct pollfd& pfd = _pollfds[idx]; (void)pfd;
  assert(pfd.fd == -channel->fd() - 1 && pfd.events == channel->events());
  size_t n = _channels.erase(channel->fd());
  assert(n == 1); (void)n;
  if (implicit_cast<size_t>(idx) == _pollfds.size() - 1) {
    _pollfds.pop_back();
  } else {
    int channelAtEnd = _pollfds.back().fd;
    iter_swap(_pollfds.begin() + idx, _pollfds.end() - 1);
    if (channelAtEnd < 0) {
      channelAtEnd = -channelAtEnd - 1;
    }
    _channels[channelAtEnd]->set_index(idx);
    _pollfds.pop_back();
  }
}
