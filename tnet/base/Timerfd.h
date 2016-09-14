#ifndef TNET_BASE_TIMERFD_H
#define TNET_BASE_TIMERFD_H

#include <tnet/base/nocopyable.h>
#include <chrono>
#include <atomic>

namespace tnet {

class TimerFd : tnet::nocopyable {
 public:
  TimerFd();
  ~TimerFd();

  int getFd() const;
  void setTime(const std::chrono::milliseconds& timeout);
 private:
  int _pipeFd[2];
  std::atomic<bool> _isTiming;
};

}  // namespace tnet

#endif  // TNET_BASE_TIMERFD_H
