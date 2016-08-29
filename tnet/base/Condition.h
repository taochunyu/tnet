#ifndef TNET_BASE_CONDITION_H
#define TNET_BASE_CONDITION_H

#include <tnet/base/Mutex.h>
#include <tnet/base/nocopyable.h>
#include <pthread.h>
#include <errno.h>
#include <inttypes.h>
#include <sys/time.h>

namespace tnet {

class Condition : tnet::nocopyable {
 public:
  explicit Condition(MutexLock &mutex) : _mutex(mutex) {
    returnCheck(pthread_cond_init(&_pcond, nullptr), "init");
  }
  ~Condition() {
    returnCheck(pthread_cond_destroy(&_pcond), "destory");
  }
  void wait() {
    MutexLock::UnassignGuard ug(_mutex);
    returnCheck(pthread_cond_wait(&_pcond, _mutex.getPthreadMutex()), "wait");
  }
  // returns true if time out, false otherwise.
  bool waitForSeconds(double seconds) {
    struct timespec abstime;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    abstime.tv_sec = tv.tv_sec;
    abstime.tv_nsec = tv.tv_usec * 1000;
    const int64_t kNanoSecondsPerSeond = 1e9;
    int64_t nanoSeconds = static_cast<int64_t>(seconds * kNanoSecondsPerSeond);
    abstime.tv_sec
      += static_cast<time_t>((abstime.tv_nsec + nanoSeconds) / kNanoSecondsPerSeond);
    abstime.tv_nsec
      = static_cast<long>((abstime.tv_nsec + nanoSeconds) % kNanoSecondsPerSeond);

    MutexLock::UnassignGuard ug(_mutex);
    return
      ETIMEDOUT == pthread_cond_timedwait(&_pcond, _mutex.getPthreadMutex(), &abstime);
  }
  void notify() {
    returnCheck(pthread_cond_signal(&_pcond), "signal");
  }
  void notifyAll() {
    returnCheck(pthread_cond_broadcast(&_pcond), "broadcast");
  }
  static void returnCheck(int ret, const char *msg = "") {
    if (ret != 0) {
      fprintf(stderr, "%s error in pthread condition: %s\n", msg, strerror(ret));
      assert(ret == 0);
    }
  }
 private:
  MutexLock &_mutex;
  pthread_cond_t _pcond;
};

}

#endif  // TNET_BASE_CONDITION_H
