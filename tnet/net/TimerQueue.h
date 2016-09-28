#ifndef TNET_NET_TIMERQUEUE_H
#define TNET_NET_TIMERQUEUE_H

#include <tnet/base/nocopyable.h>
#include <tnet/net/Callbacks.h>
#include <tnet/net/Channel.h>
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
class Timer;
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
  using TimerPair = std::pair<Timestamp, std::shared_ptr<Timer>>;
  using TimerPairSet = std::set<TimerPair>;
  using TimerSet = std::set<std::shared_ptr<Timer>>;

  void addTimerInLoop(const std::shared_ptr<Timer>& timer);
  void cancelInLoop(const std::shared_ptr<Timer>& timer);
  void handleRead();

  std::vector<TimerPair> getExpired(const Timestamp now);
  void reset(const std::vector<TimerPair>& expired, Timestamp now);

  bool insert(const std::shared_ptr<Timer>& timer);

  EventLoop* _loop;
  const int _timerfd;
  Channel _timerfdChannel;
  TimerPairSet _timers;

  bool _callingExpiredTimers;
  TimerSet _cancelingTimers;

};

}  // namespace net
}  // namespace tnet

#endif
