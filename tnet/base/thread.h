#ifndef TNET_BASE_THREAD_H
#define TNET_BASE_THREAD_H

#include <tnet/base/Types.h>
#include <tnet/base/nocopyable.h>
#include <sys/types.h>
#include <pthread.h>
#include <functional>
#include <memory>
#include <string>
#include <atomic>

namespace tnet {

class Thread : tnet::nocopyable {
 public:
  using ThreadFunc = std::function<void()>;
  explicit Thread(const ThreadFunc&, const std::string&);
  explicit Thread(ThreadFunc&&, const std::string&);
  ~Thread();
  void start();
  int join();
  bool started() const { return _started; }
  static int numCreated() { return _numCreated.load(std::memory_order_relaxed); }
 private:
  static std::atomic<int> _numCreated;
  void setDefaultName();
  bool _started;
  bool _joined;
  pthread_t _threadId;
  std::shared_ptr<pid_t> _tid;
  ThreadFunc _func;
  std::string _name;
};

}  // namespace tnet

#endif  // TNET_BASE_THREAD_H
