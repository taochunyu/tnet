#ifndef TNET_NET_CHANNEL_H
#define TNET_NET_CHANNEL_H
#include <tnet/base/nocopyable.h>
#include <functional>
namespace tnet {
namespace net {

class EventLoop;
// A selectable I/O Channel
// This class doesn't own the file descriptor.
// The file descriptor could be a socket,
// an eventfd, a timerfd, or a signalfd
class Channel : tnet::nocopyable {
 public:
  using EventCallback = std::function<void()>;

  Channel(EventLoop *loop, int fd);
  ~Channel();

  void handleEvent();
  void setReadCallback(const EventCallback &cb) {
    _readCallback = cb;
  }
  void setWriteCallback(const EventCallback &cb) {
    _writeCallback = cb;
  }
  void setErrorCallback(const EventCallback &cb) {
    _errorCallback = cb;
  }

  int fd() const { return _fd; }
  int events() const { return _events; }
  void set_revents(int revt) { _revents = revt; }
  bool isNoneEvent() const { return _events == kNoneEvent; }

  void enableReading() {
    _events |= kReadEvent;
    update();
  }
  void enableWriting() {
    _events |= kWriteEvent;
    update();
  }
  void disableReading() {
    _events &= ~kWriteEvent;
    update();
  }
  void disableAll() {
    _events = kNoneEvent;
    update();
  }
  // for Poller
  int index() const { return _index; }
  void set_index(int idx) { _index = idx; }

  EventLoop* ownerLoop() const { return _loop; }
 private:
  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

  void update();

  EventLoop* _loop;
  const int _fd;
  int _events;
  int _revents;  // current event
  int _index;

  EventCallback _readCallback;
  EventCallback _writeCallback;
  EventCallback _errorCallback;
};

}  // namespace net
}  // namespace tnet
#endif  // TNET_NET_CHANNEL_H
