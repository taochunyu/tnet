#ifndef TNET_NET_TIMERQUEUE_H
#define TNET_NET_TIMERQUEUE_H

#include <tnet/base/nocopyable.h>
#include <tnet/net/Callbacks.h>
#include <tnet/net/Channel.h>
#include <tnet/net/Timer.h>
#include <tnet/net/EventLoop.h>
#include <tnet/base/Timestamp.h>
#include <memory>
#include <vector>
#include <set>
#include <utility>

using namespace tnet::net;

namespace tnet {
namespace net {

class EventLoop;
class TimerId;

class TimerQueue : tnet::nocopyable {
 public:
  TimerQueue(EventLoop* loop);
  ~TimerQueue();

  TimerId addTimer(const TimerCallback& cb,
                   Timestamp when,
                   double interval);

  TimerId addTimer(const TimerCallback&& cb,
                   Timestamp when,
                   double interval);

  void cancel(TimerId timerId);
 private:
  static bool cmp(const std::shared_ptr<Timer>& lhs,
                  const std::shared_ptr<Timer>& rhs) {
    return lhs->expiration() < rhs->expiration();
  }
  using TimerSet = std::multiset<std::shared_ptr<Timer>, decltype(cmp)*>;
  using TimerVector = std::vector<std::shared_ptr<Timer>>;

  void addTimerInLoop(const std::shared_ptr<Timer>& timer);
  void cancelInLoop(const std::shared_ptr<Timer>& timer);
  void handleRead();

  TimerVector getExpired(const Timestamp now);
  void reset(const TimerVector& expired, Timestamp now);
  bool insert(const std::shared_ptr<Timer>& timer);


  EventLoop* _loop;
  const int _timerfd;
  Channel _timerfdChannel;
  TimerSet _timers;

  bool _callingExpiredTimers;
  TimerSet _cancelingTimers;
};

}  // namespace net
}  // namespace tnet

#endif
