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
  static int writeToPipe(int64_t howMany);
  static std::function<bool(int64_t, int64_t)> cmp;

  void pushToHeap(int64_t msExpirationAbs);
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
      ::poll(nullptr, 0, 1);
      if (!_heap.empty()) {
        int64_t msNowAbs = getMsNowAbs();
        int64_t howMany = popFromHeapUtil(msNowAbs);
        if (howMany != 0) writeToPipe(howMany);
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
  pushToHeap(msExpirationAbs);
}

void TimerFd::pushToHeap(int64_t msExpirationAbs) {
  std::lock_guard<std::mutex> lck(_mtx);
  _heap.push_back(msExpirationAbs);
  std::push_heap(_heap.begin(), _heap.end(), cmp);
}

int TimerFd::popFromHeapUtil(int64_t msNowAbs) {
  std::lock_guard<std::mutex> lck(_mtx);
  int count = 0;
  while (!_heap.empty() && _heap.front() < msNowAbs) {
    std::pop_heap(_heap.begin(), _heap.end(), cmp);
    ++count;
  }
  return count;
}

TimerFd timerfd;

}  // namespace timerfd
}  // namespace tnet
