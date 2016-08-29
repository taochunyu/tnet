#ifndef TNET_BASE_THREADPOOL_H
#define TNET_BASE_THREADPOOL_H

#include <tnet/base/nocopyable.h>
#include <tnet/base/Thread.h>
#include <tnet/base/Types.h>
#include <tnet/base/Mutex.h>
#include <tnet/base/Condition.h>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <deque>

namespace tnet {

class ThreadPool : tnet::nocopyable {
 public:
  using Task = std::function<void()>;
  explicit ThreadPool(const std::string &name = std::string("tnet ThreadPool"));
  ~ThreadPool();
  void setMaxQueueSize(int maxSize) {
    _maxQueueSize = maxSize;
  }
  void setThreadInitCallback(const Task &cb) {
    _threadInitCallback = cb;
  }
  void start(int numThreads);
  void stop();
  const std::string& name() const {
    return _name;
  }
  std::size_t queueSize() const;
  void run(const Task &f);
  void run(Task &&f);
 private:
  bool isFull() const;
  void runInThread();
  Task take();

  mutable MutexLock _mtx;
  Condition _notEmpty;
  Condition _notFull;
  std::string _name;
  Task _threadInitCallback;
  std::vector<std::shared_ptr<Thread>> _threads;
  std::deque<Task> _queue;
  std::size_t _maxQueueSize;
  bool _running;
};

}  // namespace tnet

#endif  // TNET_BASE_THREADPOOL_H
