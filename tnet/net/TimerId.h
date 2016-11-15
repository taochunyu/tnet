#ifndef TNET_NET_TIMERID_H
#define TNET_NET_TIMERID_H

#include <tnet/base/copyable.h>
#include <inttypes.h>
#include <memory>

namespace tnet {
namespace net {

class Timer;

// An opaque identifier, for canneling Timer

class TimerId : tnet::copyable {
 public:
  TimerId() {}
  TimerId(std::weak_ptr<Timer> timer, int64_t seq)
      : _timer(timer),
        _sequence(seq) {}
  friend class TimerQueue;
 private:
  std::weak_ptr<Timer> _timer;
  int64_t _sequence;
};

}  // namespace net
}  // namespace tnet

#endif  // TNET_NET_TIMERID_H
