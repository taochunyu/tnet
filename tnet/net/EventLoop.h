#ifndef TNET_NET_EVENT_LOOP_H
#define TNET_NET_EVENT_LOOP_H

#include <tnet/base/Mutex.h>
#include <tnet/base/CurrentThread.h>
#include <tnet/base/Timestamp.h>
#include <tnet/base/nocopyable.h>
#include <tnet/base/Mutex.h>
#include <tnet/net/Callbacks.h>
#include <functional>
#include <vector>
#include <memory>

namespace tnet {
namespace net {

class Channel;
class Poller;
class TimerId;
// Reactor, at most one per thread.
//
// This is an interface class, so don't expose too much details.
class EventLoop : tnet::nocopyable {
 public:
  using Functor = std::function<void()>;
  EventLoop();
  ~EventLoop();  // force out-line dtor, for scoped_ptr members.
  //
  // Loops forever.
  //
  // Must be called in the same thread as creation of the object.
  //
  void loop();
  // Quits loop.
  //
  // This is not 100% thread safe, if you call through a raw pointer,
  // better to call through shared_ptr<EventLoop> for 100% safety.
  void quit();

  void runInLoop(const Functor& cb);
  void queueInLoop(const Functor& cb);
  void runInLoop(Functor&& cb);
  void queueInLoop(Functor&& cb);

  void assertInLoopThread() {
    if (!isInLoopThread()) {
      abortNotInLoopThread();
    }
  }

  TimerId runAt(const Timestamp& time, const TimerCallback& cb);
  TimerId runAfter(const double delay, const TimerCallback& cb);
  TimerId runEvery(const double interval, const TimerCallback& cb);
  void cancel(TimerId timerId);

  TimerId runAt(const Timestamp& time, const TimerCallback&& cb);
  TimerId runAfter(const double delay, const TimerCallback&& cb);
  TimerId runEvery(const double interval, const TimerCallback&& cb);

  // internal usage
  void wakeup();
  void updateChannel(Channel* channel);
  void removeChannel(Channel* channel);

  bool isInLoopThread() const {
    return _threadId == CurrentThread::tid();
  }
  static EventLoop* getEventLoopOfCurrentThread();
 private:
  using ChannelList = std::vector<Channel*>;

  void abortNotInLoopThread();

  bool _looping;
  bool _quit;
  bool _eventHandling;
  bool _callingPengingFunctors;

  int _wakeupFd[2];

  const pid_t _threadId;
  std::unique_ptr<Poller> _poller;
  ChannelList _activeChannels;

  Channel* _currentActiveChannel;

  MutexLock _mtx;
  std::vector<Functor> _pendingFunctors;
};

}  // namespace net
}  // namespace tnet

#endif  //TNET_NET_EVENT_LOOP_H
