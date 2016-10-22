#include <tnet/base/TimerFd.h>
#include <tnet/base/nocopyable.h>
#include <tnet/base/Logging.h>
#include <sys/time.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <map>
#include <algorithm>

namespace tnet {
namespace timerfd {

class TimerFd : tnet::nocopyable {
 public:
  TimerFd();
  ~TimerFd();
  int fd() const;
  void setTime(int64_t msExpirationAbs);
 private:
  static int64_t getMsNowAbs();

  int writeToPipe();
  void push(int64_t);
  void pop();

  int64_t    _stamp = 0;
  int        _pipefd[2];
  std::mutex _mtx;
};

TimerFd::TimerFd() {
  if (::pipe(_pipefd) == -1) {
    LOG_FATAL << "create timerfd mocker failed because of pipe error";
  }
  std::thread timer([this]{
    while (true) {
      ::poll(nullptr, 0, 100);
      pop();
    }
  });
  timer.detach();
}

TimerFd::~TimerFd() {
  // LOG_FATAL << "timerfd mocker should not be destoried";
}

inline int TimerFd::fd() const {
  return _pipefd[0];
}

inline void TimerFd::setTime(int64_t msExpirationAbs) {
  printf("set: %lld\n", msExpirationAbs);
  push(msExpirationAbs);
}

inline int64_t TimerFd::getMsNowAbs() {
  struct timeval tv;
  ::gettimeofday(&tv, nullptr);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

inline int TimerFd::writeToPipe() {
  char buf[9];
  snprintf(buf, 8, "%lld", static_cast<int64_t>(1));
  return ::write(_pipefd[1], buf, sizeof(int64_t));
}

void TimerFd::push(int64_t msExpirationAbs) {
  std::lock_guard<std::mutex> lck(_mtx);
  _stamp = msExpirationAbs;
}

void TimerFd::pop() {
  std::lock_guard<std::mutex> lck(_mtx);
  //printf("call %lld %lld\n", _stamp, getMsNowAbs());
  if (_stamp > 0 && _stamp <= getMsNowAbs()) {
    writeToPipe();
    _stamp = 0;
  }

}

TimerFd timerfd;

int createTimerfd() {
  return timerfd.fd();
}

struct timespec howMuchTimeFromNow(tnet::Timestamp when) {
  int64_t microseconds = when.microSecondsSinceEpoch()
                         - tnet::Timestamp::now().microSecondsSinceEpoch();
  if (microseconds < 100) {
    microseconds = 100;
  }
  struct timespec ts;
  ts.tv_sec = static_cast<time_t>(microseconds / 1000);
  ts.tv_nsec = static_cast<long>(
      (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
  return ts;
}

void readTimerFd(int fd, tnet::Timestamp now) {
  char howMany[8];
  ::read(fd, howMany, sizeof(int64_t));
  LOG_TRACE << "TimerQueue::handleRead()" << howMany << "at" << now.toString();
}

void resetTimerFd(int fd, tnet::Timestamp expiration) {
  int64_t msNowAbs = expiration.microSecondsSinceEpoch();
  timerfd.setTime(msNowAbs);
}

}  // namespace timerfd
}  // namespace tnet
