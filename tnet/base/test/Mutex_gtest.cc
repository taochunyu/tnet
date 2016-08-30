#include <gtest/gtest.h>
#include <tnet/base/Mutex.h>
#include <tnet/base/CountDownLatch.h>
#include <tnet/base/Thread.h>
#include <tnet/base/Timestamp.h>
#include <stdio.h>
#include <functional>
#include <vector>
#include <memory>

using namespace tnet;

class MutexTest : public testing::Test {
 protected:
  virtual void SetUp() {}
  virtual void TearDown() {}
  const int kCount = 10 * 1000 * 1000;
  MutexLock g_mutex;
  std::vector<int> g_vec;
  int g_count = 0;
  void threadFunc() {
    for (int i = 0; i < kCount; i++) {
      MutexLockGuard lck(g_mutex);
      g_vec.push_back(i);
    }
  }
};

TEST_F(MutexTest, ThreadTest) {
  const int kMaxThreads = 8;
  g_vec.reserve(kMaxThreads * kCount);

  Timestamp start(Timestamp::now());
  for (int i = 0; i < kCount; i++) {
    g_vec.push_back(i);
  }
  printf("single thread without lock: %f\n", timeDifference(Timestamp::now(), start));
  start = Timestamp::now();
  threadFunc();
  printf("single thread with lock: %f\n", timeDifference(Timestamp::now(), start));
  start = Timestamp::now();
  for (int nthreads = 1; nthreads <= kMaxThreads; nthreads++) {
    std::vector<std::shared_ptr<Thread>> threads;
    g_vec.clear();
    start = Timestamp::now();
    for (int i = 0; i < nthreads; i++) {
      threads.push_back(std::make_shared<Thread>([this] { threadFunc(); }));
      threads.back() -> start();
      threads.back() -> join();
    }
    printf("%d thread(s) with lock: %f\n", nthreads, timeDifference(Timestamp::now(), start));
  }
}
