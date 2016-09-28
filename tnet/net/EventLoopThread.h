#ifndef TNET_BASE_EVENTLOOPTHREAD_H
#define TNET_BASE_EVENTLOOPTHREAD_H

#include <tnet/base/Condition.h>
#include <tnet/base/Mutex.h>
#include <tnet/base/Thread.h>
#include <tnet/base/nocopyable.h>
#include <string>

namespace tnet {
namespace net {

class EventLoop;
class EventLoopThread : tnet::nocopyable {
 public:
  using ThreadInitCallback = std::function<void(EventLoop*)>;
  EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                  const std::string& name = std::string());
  ~EventLoopThread();
  EventLoop* startLoop();
 private:
  void threadFunc();

  EventLoop* _loop;
  bool _exiting;
  Thread _thread;
  MutexLock _mtx;
  Condition _cond;
  ThreadInitCallback _cb;
};

}  // namespace net
}  // namespace tnet

#endif  // TNET_BASE_EVENTLOOPTHREAD_H
