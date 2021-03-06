#ifndef TNET_NET_CHANNEL_H
#define TNET_NET_CHANNEL_H
#include <tnet/base/nocopyable.h>
#include <tnet/base/Timestamp.h>
#include <stdio.h>
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
  using ReadEventCallback = std::function<void(Timestamp)>;

  Channel(EventLoop *loop, int fd);
  ~Channel() {}

  void handleEvent(Timestamp receiveTime);
  void onReadable(const ReadEventCallback& cb) {
    _readCallback = cb;
  }
  void onReadable(ReadEventCallback&& cb) {
    _readCallback = std::move(cb);
  }
  void onWritable(const EventCallback &cb) {
    _writeCallback = cb;
  }
  void onWritable(EventCallback&& cb) {
    _writeCallback = std::move(cb);
  }
  void onClose(const EventCallback& cb) {
    _closeCallback = cb;
  }
  void onClose(EventCallback&& cb) {
    _closeCallback = std::move(cb);
  }
  void onError(const EventCallback &cb) {
    _errorCallback = cb;
  }
  void onError(EventCallback&& cb) {
    _errorCallback = std::move(cb);
  }

  void tie(const std::shared_ptr<void>&);
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
    _events &= ~kReadEvent;
    update();
  }
  void disableWriting() {
    _events &= ~kWriteEvent;
    update();
  }
  void disableAll() {
    _events = kNoneEvent;
    update();
  }
  bool isWriting() const {
    return _events & kWriteEvent;
  }
  bool isReading() const {
    return _events & kReadEvent;
  }
  // for Poller
  int index() const { return _index; }
  void set_index(int idx) { _index = idx; }

  EventLoop* ownerLoop() const { return _loop; }
  void remove();

  std::string reventsToString() const;
  std::string eventsToString() const;
  static std::string eventsToString(int fd, int ev);
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

  bool _addedToLoop;
  std::weak_ptr<void> _tie;
  bool _tied;
  ReadEventCallback _readCallback;
  EventCallback _writeCallback;
  EventCallback _errorCallback;
  EventCallback _closeCallback;
};

}  // namespace net
}  // namespace tnet
#endif  // TNET_NET_CHANNEL_H
