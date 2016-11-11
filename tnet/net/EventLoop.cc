#include <tnet/net/EventLoop.h>
#include <tnet/base/Logging.h>
#include <tnet/base/Mutex.h>
#include <tnet/net/Channel.h>
#include <tnet/net/Poller.h>
#include <tnet/net/TimerId.h>
#include <tnet/net/TimerQueue.h>
#include <tnet/net/SocketsOps.h>

#include <assert.h>
#include <unistd.h>

using namespace tnet;
using namespace tnet::net;

namespace {

thread_local EventLoop *t_loopInThisThread = 0;
const int kPollTimeMs = 10000;

}  // namespace

EventLoop::EventLoop() : _looping(false),
                         _quit(false),
                         _threadId(tnet::CurrentThread::tid()),
                         _poller(Poller::newDefaultPoller(this)),
                         _timerQueue(new TimerQueue(this)) {
  if (::pipe(_wakeupFd) == -1) {
    LOG_FATAL << "create wakeupFd mocker failed because of pipe error";
  }
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
  _quit = false;
  LOG_TRACE << "EventLoop " << this << " start looping";
  while (!_quit) {
    printf("********** one loop start **********\n");
    _activeChannels.clear();
    _pollReturnTime = _poller -> poll(kPollTimeMs, &_activeChannels);
    if (Logger::logLevel() <= Logger::TRACE || true) {
      printActiveChannels();
    }
    _eventHandling = true;
    for (auto it = _activeChannels.begin(); it != _activeChannels.end(); it++) {
      _currentActiveChannel = *it;
      _currentActiveChannel->handleEvent(_pollReturnTime);
    }
    _currentActiveChannel = nullptr;
    _eventHandling = false;
    doPendingFunctors();
  }
  LOG_TRACE << "EventLoop " << this << " stop looping";
  _looping = false;
}

void EventLoop::quit() {
  _quit = true;
}

void EventLoop::runInLoop(const Functor& cb) {
  if (isInLoopThread()) {
    cb();
  } else {
    queueInLoop(cb);
  }
}

void EventLoop::queueInLoop(const Functor& cb) {
  {
    MutexLockGuard lck(_mtx);
    _pendingFunctors.push_back(cb);
  }
  if (!isInLoopThread() || _callingPengingFunctors) {
    wakeup();
  }
}

void EventLoop::runInLoop(Functor&& cb) {
  if (isInLoopThread()) {
    cb();
  } else {
    queueInLoop(cb);
  }
}

void EventLoop::queueInLoop(Functor&& cb) {
  {
    MutexLockGuard lck(_mtx);
    _pendingFunctors.push_back(cb);
  }
  if (!isInLoopThread() || _callingPengingFunctors) {
    wakeup();
  }
}

TimerId EventLoop::runAt(const Timestamp& time, const TimerCallback& cb) {
  return _timerQueue->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runAfter(const double delay, const TimerCallback& cb) {
  Timestamp time(addTime(Timestamp::now(), delay));
  printf("%lld\n", time.microSecondsSinceEpoch());
  return runAt(time, cb);
}

TimerId EventLoop::runEvery(const double interval, const TimerCallback& cb) {
  Timestamp time(addTime(Timestamp::now(), interval));
  return _timerQueue->addTimer(cb, time, interval);
}

TimerId EventLoop::runAt(const Timestamp& time, TimerCallback&& cb) {
  return _timerQueue->addTimer(std::move(cb), time, 0.0);
}

TimerId EventLoop::runAfter(const double delay, TimerCallback&& cb) {
  Timestamp time(addTime(Timestamp::now(), delay));
  return runAt(time, std::move(cb));
}

TimerId EventLoop::runEvery(const double interval, TimerCallback&& cb) {
  Timestamp time(addTime(Timestamp::now(), interval));
  return _timerQueue->addTimer(std::move(cb), time, interval);
}

void EventLoop::cancel(TimerId timerId) {
  return _timerQueue->cancel(timerId);
}

void EventLoop::updateChannel(Channel* channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  _poller->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  if (_eventHandling) {
    assert( (_currentActiveChannel == channel) || (std::find(_activeChannels.begin(), _activeChannels.end(), channel) == _activeChannels.end()) );
    _poller->removeChannel(channel);
  }
}

EventLoop* EventLoop::getEventLoopOfCurrentThread() {
  return t_loopInThisThread;
}

void EventLoop::abortNotInLoopThread() {
  LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
            << " was created in threadId = " << _threadId
            << ", current thread id = " <<  CurrentThread::tid();
}

void EventLoop::wakeup() {
  uint64_t one = 1;
  ssize_t n = sockets::write(_wakeupFd[1], &one, sizeof(one));
  if (n != sizeof(one)) {
    LOG_SYSERR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
  }
}

void EventLoop::handleRead() {
  uint64_t one = 1;
  ssize_t n = sockets::read(_wakeupFd[1], &one, sizeof(one));
  if (n != sizeof(one)) {
    LOG_SYSERR << "EventLoophandleRead() writes " << n << " bytes instead of 8";
  }
}

void EventLoop::doPendingFunctors() {
  std::vector<Functor> functions;
  _callingPengingFunctors = true;
  {
    MutexLockGuard lock(_mtx);
    functions.swap(_pendingFunctors);
  }
  for (int i = 0; i < functions.size(); i++) {
    functions[i]();
  }
  _callingPengingFunctors = false;
}

void EventLoop::printActiveChannels() const {
  for (auto it = _activeChannels.begin(); it != _activeChannels.end(); it++) {
    LOG_INFO << "{" << (*it)->reventsToString() << "} ";
  }
}
