#include <tnet/net/TimerQueue.h>
#include <tnet/net/TimerId.h>
#include <tnet/net/Callbacks.h>
#include <tnet/base/TimerFd.h>
#include <unistd.h>
#include <assert.h>
#include <memory>
#include <utility>

using namespace tnet;
using namespace tnet::net;
using namespace tnet::timerfd;

TimerQueue::TimerQueue(Eventloop* loop)
    : _loop(loop),
      _timerfd(createTimerfd()),
      _timerfdChannel(loop, _timerfd),
      _timers(),
      _callingExpiredTimers() {
  _timerfdChannel.setReadCallback([this]{
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
  std::unique_ptr<Timer> timer(cb, when, interval);
  _loop->runInLoop([this]{ addTimerInLoop(timer); });
  return TimerId(timer, timer->sequence());
}

TimerId TimerQueue::addTimer(const TimerCallback&& cb,
                             Timerstamp when,
                             double interval) {
  std::unique_ptr<Timer> timer(std::move(cb), when, interval);
  _loop->runInLoop([this]{ addTimerInLoop(timer); });
  return TimerId(timer, timer->sequence());
}

void TimerQueue::cancel(Timer timerId) {
  _loop->runInLoop([this]{ cancelInLoop(timerId); });
}

void TimerQueue::addTimerInLoop(std::unique_ptr<Timer> timer) {
  _loop->assertInLoopThread();
  bool earlistChanged = insert(timer);
  if (earlistChanged) {
    resetTimerFd(_timerfd, timer->expiration());
  }
}

void TimerQueue::cannelInLoop(TimerId timerId) {
  _loop->assertInLoopThread();
  assert(_timers.size() == _activeTimers.size());
  ActiveTimer timer(timeId._timer, timerId._sequence);
  ActiveTimerSet::iterator it = _activeTimers.find(timer);
  if (it != _activeTimers.end()) {
    size_t n = _timers.erase(Entry(it->first->expiration(), it->first));
    assert(n == 1);
    activeTimers.erase(it);
  } else if (_callingExpiredTimers) {
    _cannelingTimers.insert(timer);
  }
  assert(_timers.size() == activeTimers.size());
}

void TimerQueue::handleRead() {
  _loop.assertInLoopThread();
  Timestamp now(Timestamp::now());
  readTimerFd(_timerfd, now);
  std::vector<Entry> expired = getExpired(now);
  _callingExpiredTimers = true;
  _cancelingTimes.clear();
  for (auto it = expired.begin(); it != expired.end(); it++) {
    it->second->run();
  }
  _callingExpiredTimers = false;
  reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now) {
  assert(_timers.size() == _activeTimers.size());
  std::vector<Entry> expired;
  Entry sentry(now, std::unique_ptr<Timer>(void))；
  auto end = _timers.lower_bound(sentry);
  assert(end == _timers.end() || now < end->first);
  std::copy(_timers.begin(), end, back_inserter(expired));
  _timers.erase(_timers.begin(), end);
  for (auto it = expired.begin(); it != expired.end(); ++it) {
    ActiveTimer timer(it->second, it->second->sequence());
    size_t n = _activeTimers.erase(timer);
    assert(n == 1);
  }
  assert(_timer.size() == _activeTimers.size());
  return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now) {
  Timestamp nextExpire;
  for (auto it = expired.begin(); it != expired.end(); ++it) {
    ActiveTimer timer(it->second, it->second->sequence());
    bool notCannelingTimer =
      _cannelingTimers.find(timer) == _cannelingTimers.end();

    if (it->second->repeat() && notCannelingTimer) {
      it->second->restart(now);
      insert(it->second);
    }
  }

  if (!_timers.empty()) {
    nextExpire = _timers.begin()->second->expiration();
  }

  if (nextExpire.valid()) {
    resetTimerFd(_timerfd, nextExpire);
  }
}

bool TimerQueue::insert(std::unique_ptr<Timer> timer) {
  _loop->assertInLoopThread();
  assert(_timers.size() == _activeTimers.size());
  bool earlistChanged = false;
  Timestamp when = timer->expiration();
  auto it = _timers.begin();
  if (it == _timers.end() || when < it->first) {
    earlistChanged = true;
  }
  {
    std::pair<TimerList::iterator, bool> result =
      _timers.insert(Entry(when, timer));
    assert(result.second);
  }
  {
    std::pair<ActiveTimerSet::iterator, bool> result =
      _activeTimers.insert(ActiveTimer(timer, timer->sequence()));
    assert(result.second);
  }
  assert(_timers.size() == _activeTimers.size());
  return earlistChanged;
}
