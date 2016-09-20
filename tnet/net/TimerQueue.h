#ifndef TNET_NET_TIMERQUEUE_H
#define TNET_NET_TIMERQUEUE_H

#include <tnet/base/nocopyable.h>
#include <tnet/net/Callbacks.h>
#include <tnet/net/Channel.h>
#include <tnet/base/Timestamp.h>
#include <memory>
#include <vector>
#include <utility>
#include <set>

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
  using Entry = std::pair<Timestamp, std::unique_ptr<Timer>>;
  using TimerList = std::set<Entry>;
  using ActiveTimer = std::pair<std::unique_ptr<Timer>, int64_t>;
  using ActiveTimerSet = std::set<ActiveTimer>;

  void addTimerInLoop(std::unique_ptr<Timer> timer);
  void cannelInLoop(TimerId timerId);
  void handleRead();

  std::vector<Entry> getExpired(Timestamp now);
  void reset(const std::vector<Entry>& expired, Timestamp now);

  bool insert(std::unique_ptr<Timer> timer);

  Eventloop* _loop;
  const int _timerfd;
  Channel _timerfdChannel;
  TimerList _timers;

  ActiveTimerSet _activeTimers;
  bool _callingExpiredTimers;
  ActiveTimerSet _cannelingTimers;

};

}  // namespace net
}  // namespace tnet

#endif
