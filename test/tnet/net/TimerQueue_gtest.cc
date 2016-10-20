#include <gtest/gtest.h>
#include <tnet/net/EventLoop.h>
#include <tnet/net/EventLoopThread.h>
#include <tnet/base/Thread.h>
#include <tnet/net/TimerId.h>
#include <memory>

#include <stdio.h>
#include <unistd.h>

using namespace tnet;
using namespace tnet::net;

class TimerQueueTest : public testing::Test {
 protected:
  virtual void SetUp() {
    printTid();
    ::sleep(1);
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
  _loop->runAfter(1, [this]{ print("once"); });
}
