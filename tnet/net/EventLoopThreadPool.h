#ifndef TNET_NET_EVENTLOOPTHREADPOOL_H
#define TNET_NET_EVENTLOOPTHREADPOOL_H

#include <tnet/base/nocopyable.h>
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace tnet {
namespace net {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : tnet::nocopyable {
 public:
  using ThreadInitCallback = std::function<void(EventLoop*)>;
  EventLoopThreadPool(EventLoop* baseLoop, const std::string& nameArg);
  ~EventLoopThreadPool();

  void setThreadNum(int numThreads) { _numThreads = numThreads; }
  void start(const ThreadInitCallback& cb = ThreadInitCallback());

  EventLoop* getNextLoop();

  std::vector<EventLoop*> getAllLoops();

  bool started() const {
    return _started;
  }
  std::string name() const {
    return _name;
  }
 private:
  EventLoop* _baseLoop;
  std::string _name;
  bool _started;
  int _numThreads;
  size_t _next;
  std::vector<std::shared_ptr<EventLoopThread>> _threads;
  std::vector<EventLoop*> _loops;
};

}  // namespace net
}  // namespace tnet

#endif  // TNET_NET_EVENTLOOPTHREADPOOL_H
