#include <tnet/net/EventLoop.h>
#include <tnet/base/Logging.h>
#include <assert.h>
#include <poll.h>

using namespace tnet;
using namespace tnet::net;

namespace {

thread_local EventLoop *t_loopInThisThread = 0;

}  // namespace

EventLoop::EventLoop() : _looping(false), _threadId(tnet::CurrentThread::tid()) {
  LOG_TRACE << "EventLoop created " << this << "in thread" << _threadId;
  if (t_loopInThisThread) {
    LOG_FATAL
      << "Another EventLoop " << t_loopInThisThread
      << " exists in this thread " << _threadId;
  } else {
    t_loopInThisThread = this;
  }
}

EventLoop::~EventLoop() {
  assert(!_looping);
  t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
  assert(!_looping);
  assertInLoopThread();
  _looping = true;
  ::poll(nullptr, 0, 5 * 1000);
  LOG_TRACE << "EventLoop " << this << " stop looping";
  _looping = false;
}

EventLoop* EventLoop::getEventLoopOfCurrentThread() {
  return t_loopInThisThread;
}
