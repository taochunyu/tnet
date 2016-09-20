#ifndef TNET_NET_TIMERID_H
#define TNET_NET_TIMERID_H

#include <tnet/base/copyable.h>
#include <inttypes.h>

namespace tnet {
namespace net {

class Timer;

// An opaque identifier, for canneling Timer

class TimerId : tnet::copyable {
 public:
  TimerId()
      : _timer(nullptr),
        _sequence(0) {}
  TimerId(Timer* timer, int64_t seq)
      : _timer(timer),
        _sequence(seq) {}

  friend class TimerQueue;
 private:
  Timer* _timer;
  int64_t _sequence;
};

}  // namespace net
}  // namespace tnet

#endif  // TNET_NET_TIMERID_H
