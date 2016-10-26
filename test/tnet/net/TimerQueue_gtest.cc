#include <gtest/gtest.h>
#include <tnet/net/EventLoop.h>
#include <tnet/net/EventLoopThread.h>
#include <tnet/base/Thread.h>
#include <tnet/net/TimerId.h>

#include <stdio.h>
#include <unistd.h>

using namespace tnet;
using namespace tnet::net;

class TimerQueueTest : public testing::Test {
 protected:
  virtual void SetUp() {
    printTid();
    _loop = std::make_shared<EventLoop>();
  }
  virtual void TearDown() {}
  void printTid() {
    printf("pid = %d, tid = %d\n", ::getpid(), CurrentThread::tid());
    printf("now: %s\n", Timestamp::now().toString().c_str());
  }
  void print(const char* msg) {
    printf("msg: %s %s\n", Timestamp::now().toString().c_str(), msg);
    if (++_cnt == 20) {
      _loop->quit();
    }
  }
  void cancel(TimerId timerId) {
    _loop->cancel(timerId);
    printf("cancelled at %s\n", Timestamp::now().toString().c_str());
  }
  int _cnt = 0;
  std::shared_ptr<EventLoop> _loop;
};

TEST_F(TimerQueueTest, EventLoopInterfaceTest) {
  print("main");
  _loop->runAfter(10, [this]{ print("Exit"); _loop->quit(); });
  _loop->runEvery(1, [this]{ print("CLOCK"); });
  _loop->runAfter(2, [this]{ print("once2"); });
  _loop->runAfter(2.5, [this]{ print("once2.5"); });
  _loop->runAfter(3.0, [this]{ print("once3"); });
  _loop->runAfter(3.5, [this]{ print("once3.5"); });
  TimerId t45 = _loop->runAfter(4.5, [this]{ print("once4.5"); });
  _loop->runAfter(4.2, [this, &t45]{ cancel(t45); });
  _loop->runAfter(4.8, [this, &t45]{ cancel(t45); });
  TimerId t3 = _loop->runEvery(3, [this]{ print("CLOCK3"); });
  _loop->runAfter(8, [this, t3]{ cancel(t3); });
  _loop->loop();
}
