#include <tnet/base/TimerFd.h>
#include <tnet/base/Logging.h>
#include <unistd.h>
#include <poll.h>
#include <thread>

namespace tnet {

TimerFd::TimerFd() : _isTiming(false) {
  if (::pipe(_pipeFd) == -1) {
    LOG_FATAL << "TimerFd is not available";
  }
}

TimerFd::~TimerFd() {
  ::close(_pipeFd[0]);
  ::close(_pipeFd[1]);
}

int TimerFd::getFd() const {
  return _pipeFd[0];
}

void TimerFd::setTime(const std::chrono::milliseconds& timeout) {
  if (_isTiming) {
    LOG_ERROR << "Timer is running";
    return;
  }
  auto timeoutMs = static_cast<int>(timeout.count());
  _isTiming = true;
  std::thread timer([this, timeoutMs]() {
    ::poll(nullptr, 0, timeoutMs);
    ::write(_pipeFd[1], " ", 1);
    _isTiming = false;
  });
  timer.detach();
}

}  // namespace tnet
