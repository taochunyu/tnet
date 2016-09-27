#ifndef TNET_NET_TIMERQUEUE_H
#define TNET_NET_TIMERQUEUE_H

#include <tnet/base/nocopyable.h>
#include <tnet/net/Callbacks.h>
#include <tnet/net/Channel.h>
#include <tnet/base/Timestamp.h>
#include <memory>
#include <vector>
#include <set>
#include <utility>

namespace tnet {
namespace net {

class Eventloop;
class Timer;
class TimerId;

class TimerQueue : tnet::nocopyable {
 public:
  TimerQueue(Eventloop* loop);
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

  void addTimerInLoop(std::shared_ptr<Timer> timer);
  void cannelInLoop(TimerId timerId);
  void handleRead();

  TimerPairSet getExpired(Timestamp now);
  void reset(const TimerPairSet& expired, Timestamp now);

  bool insert(Timer timer);

  Eventloop* _loop;
  const int _timerfd;
  Channel _timerfdChannel;
  TimerPairSet _timers;

  bool _callingExpiredTimers;
  TimerSet _cannelingTimers;

};

}  // namespace net
}  // namespace tnet

#endif
