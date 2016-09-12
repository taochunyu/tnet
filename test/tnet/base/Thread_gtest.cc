#include <gtest/gtest.h>
#include <tnet/base/Thread.h>
#include <tnet/base/CurrentThread.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <string>
#include <functional>

class ThreadTest : public testing::Test {
 protected:
  virtual void SetUp() {}
  virtual void TearDown() {}
  static void mySleep(int seconds) {
    timespec t = { seconds, 0 };
    ::nanosleep(&t, nullptr);
  }
  static void threadFunc1() {
    printf("tid = %d\n", tnet::CurrentThread::tid());
  }
  static void threadFunc2(int x) {
    printf("tid = %d, x = %d\n", tnet::CurrentThread::tid(), x);
  }
  static void threadFunc3() {
    printf("tid = %d\n", tnet::CurrentThread::tid());
    mySleep(1);
  }
  class Foo {
   public:
    explicit Foo(double x) : _x(x) {}
    void memberFunc1() {
      printf("tid = %d, Foo::_x = %f\n", tnet::CurrentThread::tid(), _x);
    }
    void memberFunc2(const std::string &test) {
      printf("tid = %d, Foo::_x = %f, test = %s\n",
        tnet::CurrentThread::tid(), _x, test.c_str());
    }
    double _x;
  };
};

TEST_F(ThreadTest, CreateTest) {
  printf("pid = %d, tid = %d\n", ::getpid(), tnet::CurrentThread::tid());
  tnet::Thread t1(threadFunc1);
  t1.start();
  t1.join();

  tnet::Thread t2([]{ threadFunc2(42); }, "thread for free function with argument");
  t2.start();
  t2.join();

  Foo foo(87.53);
  tnet::Thread t3([&foo]{ foo.memberFunc1(); },
    "thread for member function without argument");
  t3.start();
  t3.join();

  tnet::Thread t4([&foo]{ foo.memberFunc2("hello"); });
  t4.start();
  t4.join();

  {
    tnet::Thread t5(threadFunc3);
    t5.start();
  }
  mySleep(2);
  {
    tnet::Thread t6(threadFunc3);
    t6.start();
  }
  mySleep(2);
  printf("number of created threads %d\n", tnet::Thread::numCreated());

}
