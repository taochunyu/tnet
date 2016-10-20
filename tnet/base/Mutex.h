#ifndef TNET_BASE_MUTEX_H
#define TNET_BASE_MUTEX_H

#include <tnet/base/CurrentThread.h>
#include <tnet/base/nocopyable.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

namespace tnet {

class MutexLock : tnet::nocopyable {
  friend class Condition;
 public:
  MutexLock() : _holder(0) {
    returnCheck(::pthread_mutex_init(&_mutex, nullptr), "init");
  }
  ~MutexLock() {
    assert(_holder == 0);
    returnCheck(::pthread_mutex_destroy(&_mutex), "destory");
  }
  bool isLockedByThisThread() const {
    return _holder == CurrentThread::tid();
  }
  void assertLocked() const {
    assert(isLockedByThisThread());
  }
  void lock() {
    returnCheck(::pthread_mutex_lock(&_mutex), "lock");
    assignHolder();
  }
  void unlock() {
    unassignHolder();
    returnCheck(::pthread_mutex_unlock(&_mutex), "unlock");
  }
  pthread_mutex_t* getPthreadMutex() /*no-const*/{
    return &_mutex;
  }
  static void returnCheck(int ret, const char* msg = "") {
    if (ret != 0) {
      fprintf(stderr, "%s error in pthread mutex: %s\n", msg, strerror(ret));
      assert(ret == 0);
    }
  }
 private:
  class UnassignGuard : tnet::nocopyable {
   public:
    UnassignGuard(MutexLock &owner) : _owner(owner) {
      _owner.unassignHolder();
    }
    ~UnassignGuard() {
      _owner.assignHolder();
    }
   private:
    MutexLock &_owner;
  };
  void assignHolder() {
    _holder = CurrentThread::tid();
  }
  void unassignHolder() {
    _holder = 0;
  }
  pthread_mutex_t _mutex;
  pid_t _holder;
};

class MutexLockGuard : tnet::nocopyable {
 public:
  explicit MutexLockGuard(MutexLock &mutex) : _mutex(mutex) {
    _mutex.lock();
  }
  ~MutexLockGuard() {
    _mutex.unlock();
  }
 private:
  MutexLock& _mutex;
};

}  // namespace tnet

#define MutexLockGuard(x) error "Missing guard object name"

#endif  // TNET_BASE_MUTEX_H
