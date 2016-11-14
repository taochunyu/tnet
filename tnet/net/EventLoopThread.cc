#include <tnet/net/EventLoopThread.h>
#include <tnet/net/EventLoop.h>

using namespace tnet;
using namespace tnet::net;

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
                                const std::string& name)
    : _loop(nullptr),
      _exiting(false),
      _thread([this] { threadFunc(); }, name),
      _mtx(),
      _cond(_mtx),
      _cb(cb) {}

EventLoopThread::~EventLoopThread() {
  _exiting = true;
  if (_loop != nullptr) {
    _loop->quit();
    _thread.join();
  }
}

EventLoop* EventLoopThread::startLoop() {
  assert(!_thread.started());
  _thread.start();
  {
    MutexLockGuard lck(_mtx);
    while (_loop == nullptr) {
      _cond.wait();
    }
  }
  return _loop;
}

void EventLoopThread::threadFunc() {
  EventLoop loop;
  if (_cb) {
    _cb(&loop);
  }
  {
    MutexLockGuard lck(_mtx);
    _loop = &loop;
    _cond.notify();
  }
  loop.loop();
  _loop = nullptr;
}
