#include <tnet/base/TimerFd.h>
#include <tnet/base/nocopyable.h>
#include <tnet/base/Logging.h>
#include <unistd.h>
#include <poll.h>
#include <stdio.h>
#include <sys/time.h>
#include <thread>
#include <mutex>
#include <map>
#include <set>

namespace tnet {
namespace timerfd {

class TimerFd : tnet::nocopyable {
 public:
  TimerFd() {}
  ~TimerFd() {
    std::for_each(_map.begin(), _map.end(), [](std::pair<int, OneFdGroup&> g) {
      ::close(g.second.timerfd[0]);
      ::close(g.second.timerfd[1]);
    });
  }
  int create() {
    int pipeFd[2];
    if (::pipe(pipeFd)  == -1) {
      LOG_FATAL << "timerfd create failed because of pipe error";
    }
    OneFdGroup group(pipeFd);
    _map.insert({ pipeFd[0], std::ref(group) });
    std::thread timer([&group]{
      while (true) {
        ::poll(nullptr, 0, 1);
        std::lock_guard<std::mutex> lck(group.mtx);
        if (!group.willHandle.empty()) {
          struct timeval tv;
          ::gettimeofday(&tv, nullptr);
          int64_t absMs = tv.tv_sec * 1000 + tv.tv_usec / 1000;
          auto beg = group.willHandle.begin();
          auto psi = group.willHandle.upper_bound(absMs);
          if (psi != beg) {
            char buf[8];
            int64_t many = 0;
            for (auto t = beg; t != psi; ++t) {
              ++many;
            }
            snprintf(buf, 8, "%lld", many);
            ::write(group.timerfd[1], buf, sizeof(int64_t));
            group.willHandle.erase(group.willHandle.begin(), psi);
          }
        }
      }
    });
    timer.detach();
    return pipeFd[0];
  }
  void close(int timerfd) {
    auto group = _map.find(timerfd);
    if (group == _map.end()) {
      LOG_FATAL << "this is not a timerfd";
      return;
    }
    ::close(group->second.timerfd[0]);
    ::close(group->second.timerfd[1]);
  }
  void setTime(int timerfd, int absMs) {
    auto group_iter = _map.find(timerfd);
    if (group_iter == _map.end()) {
      LOG_FATAL << "this is not a timerfd";
      return;
    }
    std::lock_guard<std::mutex> lck((group_iter->second).mtx);
    group_iter->second.willHandle.emplace(absMs);
  }
 private:
  struct OneFdGroup {
    OneFdGroup(int* fd) {
      timerfd[0] = fd[0];
      timerfd[1] = fd[1];
    }
    int timerfd[2];
    std::multiset<int64_t> willHandle;
    std::mutex mtx;
  };
  std::map<int, OneFdGroup&> _map;
};

TimerFd timerfd;

}  // namespace timerfd
}  // namespace tnet
