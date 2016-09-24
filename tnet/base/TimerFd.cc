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
  static std::function<bool(int64_t, int64_t)> cmp;

  void pushToHeap(int64_t msExpirationAbs);
  int writeToPipe(int64_t howMany);
  int popFromHeapUtil(int64_t msNowAbs);

  int                  _pipefd[2];
  std::vector<int64_t> _heap;
  std::mutex           _mtx;
};

std::function<bool(int64_t, int64_t)>
TimerFd::cmp = [](int64_t l, int64_t r) { return l > r; };

TimerFd::TimerFd() {
  if (::pipe(_pipefd) == -1) {
    LOG_FATAL << "timerfd create failed because of pipe error";
  }
  std::make_heap(_heap.begin(), _heap.end(), cmp);
  std::thread timer([this]{
    while (true) {
      ::poll(nullptr, 0, 100);
      if (!_heap.empty()) {
        int64_t msNowAbs = getMsNowAbs();
        printf("%lld\n", msNowAbs);
        if (_heap.front() < msNowAbs) {
          int64_t howMany = popFromHeapUtil(msNowAbs);
          if (howMany != 0) writeToPipe(howMany);
        }
      }
    }
  });
  timer.detach();
}

TimerFd::~TimerFd() {
  LOG_FATAL << "timerfd mocker should not be destoried";
}

inline int TimerFd::fd() const {
  return _pipefd[0];
}

inline void TimerFd::setTime(int64_t msExpirationAbs) {
  printf("set: %lld\n", msExpirationAbs);
  pushToHeap(msExpirationAbs);
}

inline int64_t TimerFd::getMsNowAbs() {
  struct timeval tv;
  ::gettimeofday(&tv, nullptr);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

inline int TimerFd::writeToPipe(int64_t howMany) {
  char buf[9];
  snprintf(buf, 8, "%lld", howMany);
  return ::write(_pipefd[1], buf, sizeof(int64_t));
}

void TimerFd::pushToHeap(int64_t msExpirationAbs) {
  std::lock_guard<std::mutex> lck(_mtx);
  _heap.push_back(msExpirationAbs);
  std::push_heap(_heap.begin(), _heap.end(), cmp);
}

int TimerFd::popFromHeapUtil(int64_t msNowAbs) {
  printf("lock\n");
  std::lock_guard<std::mutex> lck(_mtx);
  int count = 0;
  while (!_heap.empty() && _heap.front() < msNowAbs) {
    std::pop_heap(_heap.begin(), _heap.end(), cmp);
    _heap.pop_back();
    ++count;
    printf("%d\n", (int)_heap.size());
  }
  return count;
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
