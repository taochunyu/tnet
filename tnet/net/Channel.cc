#include <tnet/net/Channel.h>
#include <tnet/net/EventLoop.h>
#include <tnet/base/Logging.h>
#include <tnet/base/Timestamp.h>
#include <poll.h>

#include <sstream>
#include <string>

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

void Channel::handleEvent(Timestamp receiveTime) {
  LOG_INFO << reventsToString();
  if (_revents & POLLNVAL) {
    LOG_WARN << "Channel::handle_event() POLLNVAL";
  }
  if (_revents & (POLLERR | POLLNVAL)) {
    if (_errorCallback) _errorCallback();
  }
  if (_revents & (POLLIN | POLLPRI | POLLRDHUP)) {
    if (_readCallback) _readCallback(receiveTime);
  }
  if (_revents & POLLOUT) {
    if (_writeCallback) _writeCallback();
  }
  if ((_revents & POLLHUP) && !(_revents & POLLIN)) {
    printf("close call\n");
    if (_closeCallback) _closeCallback();
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

std::string Channel::reventsToString() const {
  return eventsToString(_fd, _revents);
}

std::string Channel::eventsToString() const {
  return eventsToString(_fd, _events);
}

std::string Channel::eventsToString(int fd, int ev) {
  std::ostringstream oss;
  oss << fd << ": ";
  if (ev & POLLIN)
    oss << "IN ";
  if (ev & POLLPRI)
    oss << "PRI ";
  if (ev & POLLOUT)
    oss << "OUT ";
  if (ev & POLLHUP)
    oss << "HUP ";
  if (ev & POLLRDHUP)
    oss << "RDHUP ";
  if (ev & POLLERR)
    oss << "ERR ";
  if (ev & POLLNVAL)
    oss << "NVAL ";

  return oss.str();
}
