#ifndef TNET_BASE_COUNT_DOWN_LATCH_H
#define TNET_BASE_COUNT_DOWN_LATCH_H

#include <tnet/base/Mutex.h>
#include <tnet/base/Condition.h>
#include <tnet/base/nocopyable.h>

namespace tnet {

class CountDownLatch : tnet::nocopyable {
 public:
  explicit CountDownLatch(int count)
    : _mutex(), _condition(_mutex), _count(count) {}
  void wait() {
    MutexLockGuard lck(_mutex);
    while (_count > 0) {
      _condition.wait();
    }
  }
  void countDown() {
    MutexLockGuard lck(_mutex);
    printf("hello\n");
    --_count;
    if (_count == 0) {
      printf("world\n");
      _condition.notifyAll();
    }
  }
  int getCount() const {
    MutexLockGuard lck(_mutex);
    return _count;
  }
 private:
  mutable MutexLock _mutex;
  Condition _condition;
  int _count;
};

}

#endif  // TNET_BASE_COUNT_DOWN_LATCH_H
