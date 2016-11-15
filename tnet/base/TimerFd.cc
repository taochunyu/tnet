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
#include <cassert>
#include <memory>

using namespace tnet;
using namespace tnet::timerfd;

class TimerFd : tnet::nocopyable {
  friend class TimerFdManager;
 public:
  TimerFd();
  int fd() const;
  void setTime(int64_t msExpirationAbs);
 private:
  static int64_t getMsNowAbs();

  int writeToPipe();
  void pop();

  int64_t    _stamp = 0;
  int        _pipefd[2];
  std::mutex _mtx;
};

class TimerFdManager : tnet::nocopyable {
 public:
  TimerFdManager();
  int createNewTimerFd();
  void resetTimerFd(int fd, int64_t msNowAbs);
 private:
  static std::map<int, std::unique_ptr<TimerFd>> _timerFds;
  void checkTimerFds();
  std::mutex _mtx;
};

std::map<int, std::unique_ptr<TimerFd>> TimerFdManager::_timerFds;

TimerFdManager::TimerFdManager() {
  std::thread timer([this]{
    while (true) {
      ::poll(nullptr, 0, 100);
      checkTimerFds();
    }
  });
  timer.detach();
}

int TimerFdManager::createNewTimerFd() {
  std::unique_ptr<TimerFd> p(new TimerFd());
  int fd = p->fd();
  {
    std::lock_guard<std::mutex> lck(_mtx);
    _timerFds.emplace(fd, std::move(p));
  }
  return fd;
}

void TimerFdManager::resetTimerFd(int fd, int64_t msNowAbs) {
  std::lock_guard<std::mutex> lck(_mtx);
  assert(_timerFds.find(fd) != _timerFds.end());
  _timerFds[fd]->setTime(msNowAbs);
}

void TimerFdManager::checkTimerFds() {
  for (auto &it : _timerFds) {
    it.second->pop();
  }
}


TimerFd::TimerFd() {
  if (::pipe(_pipefd) == -1) {
    LOG_FATAL << "create timerfd mocker failed because of pipe error";
  }
}

int TimerFd::fd() const {
  return _pipefd[0];
}

void TimerFd::setTime(int64_t msExpirationAbs) {
  std::lock_guard<std::mutex> lck(_mtx);
  _stamp = msExpirationAbs;
}

int64_t TimerFd::getMsNowAbs() {
  struct timeval tv;
  ::gettimeofday(&tv, nullptr);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int TimerFd::writeToPipe() {
  int readFd = _pipefd[0];
  return ::write(_pipefd[1], &readFd, sizeof(readFd));
}

void TimerFd::pop() {
  std::lock_guard<std::mutex> lck(_mtx);
  if (_stamp > 0 && _stamp <= getMsNowAbs()) {
    writeToPipe();
    _stamp = 0;
  }

}

TimerFdManager __TNET__timerFdManager;

int tnet::timerfd::createTimerfd() {
  return __TNET__timerFdManager.createNewTimerFd();
}

void tnet::timerfd::readTimerFd(int fd, Timestamp now) {
  int readFd;
  ::read(fd, &readFd, sizeof(readFd));
  assert(fd == readFd);
}

void tnet::timerfd::resetTimerFd(int fd, Timestamp expiration) {

  int64_t msNowAbs = expiration.microSecondsSinceEpoch();
  __TNET__timerFdManager.resetTimerFd(fd, msNowAbs);
}




struct timespec howMuchTimeFromNow(Timestamp when) {
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
