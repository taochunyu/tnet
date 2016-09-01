#ifndef TNET_NET_EVENT_LOOP_H
#define TNET_NET_EVENT_LOOP_H

#include <tnet/base/Mutex.h>
#include <tnet/base/CurrentThread.h>
#include <tnet/base/Timestamp.h>
#include <tnet/base/nocopyable.h>
#include <tnet/net/Callbacks.h>
#include <functional>
#include <memory>

namespace tnet {
namespace net {

///
/// Reactor, at most one per thread.
///
/// This is an interface class, so don't expose too much details.
class EventLoop : tnet::nocopyable {
 public:
  using Functor = std::function<void()>;
  EventLoop();
  ~EventLoop();  // force out-line dtor, for scoped_ptr members.
  ///
  /// Loops forever.
  ///
  /// Must be called in the same thread as creation of the object.
  ///
  void loop();
  /// Quits loop.
  ///
  /// This is not 100% thread safe, if you call through a raw pointer,
  /// better to call through shared_ptr<EventLoop> for 100% safety.
  void quit();
  void assertInLoopThread() {
    if (!isInLoopThread()) {
      abortNotInLoopThread();
    }
  }
  bool isInLoopThread() const {
    return _threadId == CurrentThread::tid();
  }
 private:
   void abortNotInLoopThread();
   bool _looping;
   const pid_t _threadId;
};

}  // namespace net
}  // namespace tnet

#endif  //TNET_NET_EVENT_LOOP_H
