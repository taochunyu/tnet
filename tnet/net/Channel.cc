#include <tnet/net/Channel.h>
#include <tnet/net/EventLoop.h>
#include <tnet/base/Logging.h>
#include <poll.h>

#ifndef POLLRDHUP
  const int POLLRDHUP = 0;
#endif

using namespace tnet;
using namespace tnet::net;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
  : _loop(loop), _fd(fd), _events(0), _revents(0), _index(-1) {}

void Channel::handleEvent() {
  if (_revents & POLLNVAL) {
    LOG_WARN << "Channel::handle_event() POLLNVAL";
  }
  if (_revents & (POLLERR | POLLNVAL)) {
    if (_errorCallback) _errorCallback();
  }
  if (_revents & (POLLIN | POLLPRI | POLLRDHUP)) {
    if (_readCallback) _readCallback();
  }
  if (_revents & POLLOUT) {
    if (_writeCallback) _writeCallback();
  }
}

void Channel::tie(const std::shared_ptr<void>& obj) {
  _tie = obj;
  _tied = true;
}

void Channel::update() {
  int a = this->fd();
  _loop->updateChannel(this);
  (void)a;
}

void Channel::remove() {
  assert(isNoneEvent());
  _addedToLoop = false;
  _loop->removeChannel(this);
}
