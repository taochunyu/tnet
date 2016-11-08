#include <tnet/net/TimerQueue.h>
#include <tnet/net/TimerId.h>
#include <tnet/net/Timer.h>
#include <tnet/net/Callbacks.h>
#include <tnet/net/EventLoop.h>
#include <tnet/base/TimerFd.h>
#include <tnet/base/Logging.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>
#include <memory>
#include <utility>

using namespace tnet;
using namespace tnet::net;
using namespace tnet::timerfd;

TimerQueue::TimerQueue(EventLoop* loop)
    : _loop(loop),
      _timerfd(createTimerfd()),
      _timerfdChannel(loop, _timerfd),
      _timers(cmp),
      _callingExpiredTimers(false),
      _cancelingTimers(cmp) {
  _timerfdChannel.onReadable([this](Timestamp receiveTime){
    handleRead();
  });
  _timerfdChannel.enableReading();
}

TimerQueue::~TimerQueue() {
  _timerfdChannel.disableAll();
  _timerfdChannel.remove();
  ::close(_timerfd);
}

TimerId TimerQueue::addTimer(const TimerCallback& cb,
                             Timestamp when,
                             double interval) {
  auto timer = std::make_shared<Timer>(cb, when, interval);
  _loop->runInLoop([this, &timer]{ addTimerInLoop(timer); });
  return TimerId(std::weak_ptr<Timer>(timer), timer->sequence());
}

TimerId TimerQueue::addTimer(const TimerCallback&& cb,
                             Timestamp when,
                             double interval) {
  auto timer = std::make_shared<Timer>(cb, when, interval);
  _loop->runInLoop([this, timer]{ addTimerInLoop(timer); });
  return TimerId(std::weak_ptr<Timer>(timer), timer->sequence());
}

void TimerQueue::cancel(TimerId timerId) {
  auto sp = timerId._timer.lock();
  if (sp) {
    _loop->runInLoop([this, &sp]{ cancelInLoop(sp); });
  } else {
    LOG_SYSERR << "Timer " << timerId._sequence << " has been deleted";
  }
}

void TimerQueue::addTimerInLoop(const std::shared_ptr<Timer>& timer) {
  _loop->assertInLoopThread();
  bool earlistChanged = insert(timer);
  if (earlistChanged) {
    resetTimerFd(_timerfd, timer->expiration());
  }
}

void TimerQueue::cancelInLoop(const std::shared_ptr<Timer>& timer) {
  _loop->assertInLoopThread();
  auto n = _timers.erase(timer);
  if (n == 0 && _callingExpiredTimers) {
    _cancelingTimers.insert(timer);
  }
  assert(n == 1);
}

void TimerQueue::handleRead() {
  _loop->assertInLoopThread();
  Timestamp now(Timestamp::now());
  readTimerFd(_timerfd, now);
  auto expired = getExpired(now);
  _callingExpiredTimers = true;
  _cancelingTimers.clear();
  for (auto it = expired.begin(); it != expired.end(); it++) {
    (*it)->run();
  }
  _callingExpiredTimers = false;
  reset(expired, now);
}

TimerQueue::TimerVector TimerQueue::getExpired(const Timestamp now) {
  TimerVector expired;
  auto ptr_now = std::make_shared<Timer>([]{}, now, 0.0);
  auto end = _timers.upper_bound(ptr_now);
  assert(end == _timers.end() || now < (*end)->expiration());
  std::copy(_timers.begin(), end, back_inserter(expired));
  _timers.erase(_timers.begin(), end);
  return expired;
}

void TimerQueue::reset(const TimerVector& expired, Timestamp now) {
  Timestamp nextExpire;
  for (auto it = expired.begin(); it != expired.end(); ++it) {
    bool notCannelingTimer =
      _cancelingTimers.find(*it) == _cancelingTimers.end();
    if ((*it)->repeat() && notCannelingTimer) {
      (*it)->restart(now);
      insert(*it);
    }
  }

  if (!_timers.empty()) {
    nextExpire = (*_timers.begin())->expiration();
  }

  if (nextExpire.valid()) {
    resetTimerFd(_timerfd, nextExpire);
  }
}

bool TimerQueue::insert(const std::shared_ptr<Timer>& timer) {
  _loop->assertInLoopThread();
  bool earlistChanged = false;
  Timestamp when = timer->expiration();
  auto it = _timers.begin();
  if (it == _timers.end() || when < (*it)->expiration()) {
    earlistChanged = true;
  }
  _timers.insert(timer);
  return earlistChanged;
}
