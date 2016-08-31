#include <gtest/gtest.h>
#include <tnet/base/ThreadPool.h>
#include <tnet/base/CountDownLatch.h>
#include <tnet/base/CurrentThread.h>
#include <tnet/base/Logging.h>
#include <unistd.h>
#include <stdio.h>
#include <string>

using namespace tnet;

class ThreadPoolTest : public testing::Test {
 protected:
  virtual void SetUp() {}
  virtual void TearDown() {}
  static void print() {
    printf("tid = %d\n", tnet::CurrentThread::tid());
  }
  static void printString(const std::string &str) {
    LOG_INFO << str;
    usleep(100 * 1000);
  }
  void test(int maxSize) {
    LOG_WARN << "Test ThreadPool with max queue size " << maxSize;
    ThreadPool pool("MainThreadPool");
    pool.setMaxQueueSize(maxSize);
    pool.start(5);
    /*LOG_WARN << "Adding";
    pool.run(print);
    pool.run(print);
    for (int i = 0; i < 100; i++) {
      char buf[32];
      snprintf(buf, sizeof buf, "task %d", i);
      pool.run([buf]{ printString(buf); });
    }
    LOG_WARN << "Done";*/
    CountDownLatch latch(1);
    pool.run([&latch] { latch.countDown(); });
    latch.wait();
    pool.stop();
  }
};

TEST_F(ThreadPoolTest, PoolTest) {
  //test(0);
  test(10);
  /*test(5);
  test(10);
  test(50);*/
}
