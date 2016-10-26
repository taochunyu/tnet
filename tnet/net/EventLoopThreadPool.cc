#include <tnet/net/EventLoopThreadPool.h>
#include <tnet/net/EventLoop.h>
#include <tnet/net/EventLoopThread.h>
#include <memory>

#include <stdio.h>
#include <assert.h>

using namespace tnet;
using namespace tnet::net;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop,
                                         const std::string& nameArg)
    : _baseLoop(baseLoop),
      _name(nameArg),
      _started(false),
      _numThreads(0),
      _next(0) {}

EventLoopThreadPool::~EventLoopThreadPool() {}

void EventLoopThreadPool::start(const ThreadInitCallback& cb) {
  assert(!_started);
  _baseLoop->assertInLoopThread();

  _started = true;

  for (int i = 0; i < _numThreads; i++) {
    char buf[_name.size() + 32];
    snprintf(buf, sizeof(buf), "%s%d", _name.c_str(), i);
    auto tp = std::make_shared<EventLoopThread>(cb, std::string(buf));
    _threads.push_back(tp);
    _loops.push_back(tp->startLoop());
  }
  if (_numThreads == 0 && cb) {
    cb(_baseLoop);
  }
}

EventLoop* EventLoopThreadPool::getNextLoop() {
  _baseLoop->assertInLoopThread();
  assert(_started);
  EventLoop* loop = _baseLoop;
  if (!_loops.empty()) {
    loop = _loops[_next];
    ++_next;
    if (_next >= _loops.size()) {
      _next = 0;
    }
  }
  return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops() {
  _baseLoop->assertInLoopThread();
  assert(_started);
  if (_loops.empty()) {
    return std::vector<EventLoop*>(1, _baseLoop);
  } else {
    return _loops;
  }
}
