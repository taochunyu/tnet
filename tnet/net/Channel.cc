#include <tnet/net/Channel.h>
#include <tnet/net/EventLoop.h>
#include <tnet/base/Logging.h>

using namespace tnet;
using namespace tnet::net;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
  : _loop(loop), _fd(fd), _events(0), _revents(0), _index(-1) {}
void Channel::handleEvent() {
  if (_revents & POLLNVAl) {
    Log_WARN << "Channel::handle_event() POLLNVAl";
  }
  if (_revents & (POLLERR | POLLNVAl)) {
    if (_errorCallback) _errorCallback();
  }
  if (_revents & (POLLIN | POLLPRI | POLLRDHUP)) {
    if (_readCallback) _readCallback();
  }
  if (_revents & POLLOUT) {
    if (_writeCallback) _writeCallback();
  }
}

void Channel::update() {
  _loop -> updateChannel(this);
}
