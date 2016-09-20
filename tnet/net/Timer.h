#ifndef TNET_NET_TIMER_H
#define TNET_NET_TIMER_H

#include <tnet/base/nocopyable.h>
#include <tnet/base/Timestamp.h>
#include <tnet/net/Callbacks.h>
#include <atomic>

namespace tnet {
namespace net {

class Timer : tnet::nocopyable {
 public:
  Timer(const TimerCallback& cb, Timestamp when, double interval)
      : _callback(cb),
        _expiration(when),
        _interval(interval),
        _repeat(interval > 0.0),
        _sequence(std::atomic_load(&_numCreated)) {}
  Timer(const TimerCallback&& cb, Timestamp when, double interval)
      : _callback(cb),
        _expiration(when),
        _interval(interval),
        _repeat(interval > 0.0),
        _sequence(std::atomic_load(&_numCreated)) {}

  void run() { _callback(); }
  Timestamp expiration() const { return _expiration; }
  bool repeat() const { return _repeat; }
  int64_t sequence() const { return _sequence; }

  void restart(Timestamp now);

  static int64_t numCreated() { return std::atomic_load(&_numCreated); }
 private:
  const TimerCallback _callback;
  Timestamp _expiration;
  const double _interval;
  const bool _repeat;
  const int64_t _sequence;

  static std::atomic<int64_t> _numCreated;
};

}  // namespace net
}  // namespace tnet

#endif  // TNET_NET_TIMER_H
