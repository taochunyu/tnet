#include <tnet/net/Timer.h>

using namespace tnet;
using namespace net;

std::atomic<int64_t> Timer::_numCreated;

void Timer::restart(Timestamp now) {
  if (_repeat) {
    _expiration = addTime(now, _interval);
  } else {
    _expiration = Timestamp::invalid();
  }
}
