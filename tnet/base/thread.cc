#include <tnet/base/Thread.h>
#include <tnet/base/Exception.h>
#include <tnet/base/CurrentThread.h>
#include <tnet/base/TimeStamp.h>
#include <tnet/base/Logging.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <string>
#include <memory>
#include <utility>

namespace tnet {

namespace CurrentThread {

thread_local int t_cacheTid = 0;
thread_local char t_tidString[32];
thread_local int t_tidStringLength;
thread_local const char* t_threadName = "unknown";

}  // namespace CurrentThread

namespace detail {

pid_t gettid() {
  return pthread_mach_thread_np(pthread_self());
}

void afterFork() {
  tnet::CurrentThread::t_cacheTid = 0;
  tnet::CurrentThread::t_threadName = "main";
  tnet::CurrentThread::tid();
}

class ThreadNameInitializer {
 public:
  ThreadNameInitializer() {
    tnet::CurrentThread::t_threadName = "main";
    tnet::CurrentThread::tid();
    pthread_atfork(nullptr, nullptr, &afterFork);
  }
};

class ThreadData {
 public:
  using ThreadFunc = tnet::Thread::ThreadFunc;
  ThreadFunc _func;
  std::string _name;
  std::weak_ptr<pid_t> _wkTid;
  ThreadData(const ThreadFunc &func, std::string &name, std::weak_ptr<pid_t> tid)
    : _func(func), _name(name), _wkTid(tid) {}
  void runInThread() {
    pid_t tid = tnet::CurrentThread::tid();
    std::shared_ptr<pid_t> tidp = _wkTid.lock();
    if (tidp) {
      *tidp = tid;
      tidp.reset();
    }
    tnet::CurrentThread::t_threadName = _name.empty() ? "tnetThread" : _name.c_str();
    // ::prctl(PR_SET_NAME, tnet::CurrentThread::t_threadName);  // linux only
    try {
      _func();
      tnet::CurrentThread::t_threadName = "finish";
    } catch (const Exception &ex) {
      tnet::CurrentThread::t_threadName = "crashed";
      fprintf(stderr, "Exception caught in Thread %s\n", _name.c_str());
      fprintf(stderr, "Reason: %s\n", ex.what());
      fprintf(stderr, "Stack trace: %s\n", ex.stackTrace());
      abort();
    } catch (const std::exception &ex) {
      tnet::CurrentThread::t_threadName = "crashed";
      fprintf(stderr, "Exception caught in Thread %s\n", _name.c_str());
      fprintf(stderr, "Reason: %s\n", ex.what());
      abort();
    } catch (...) {
      tnet::CurrentThread::t_threadName = "crashed";
      fprintf(stderr, "Unknown exception caught in Thread %s\n", _name.c_str());
      throw;
    }
  }
};

void* startThread(void *obj) {
  std::unique_ptr<ThreadData> data(static_cast<ThreadData*>(obj));
  data -> runInThread();
  return nullptr;
}

}  // namespace detail

void CurrentThread::cacheTid() {
  if (t_cacheTid == 0) {
    t_cacheTid = detail::gettid();
    t_tidStringLength = snprintf(t_tidString, sizeof(t_tidString), "%5d", t_cacheTid);
  }
}

bool CurrentThread::isMainThread() {
  return tid() == ::getpid();
}

void CurrentThread::sleepUsec(int64_t usec) {
  struct timespec ts = { 0, 0 };
  ts.tv_sec = static_cast<time_t>(usec / Timestamp::kMicroSecondsPerSecond);
  ts.tv_nsec = static_cast<long>(usec % Timestamp::kMicroSecondsPerSecond * 1000);
  ::nanosleep(&ts, nullptr);
}

std::atomic<int> Thread::_numCreated = ATOMIC_VAR_INIT(0);  // 不初始化会链接错误

Thread::Thread(const ThreadFunc &func)
  : _started(false), _joined(false),  _func(func)
{
  setDefaultName();
}

Thread::Thread(ThreadFunc &&func)
  : _started(false), _joined(false), _func(std::move(func))
{
  setDefaultName();
}

Thread::Thread(const ThreadFunc &func, const std::string &name)
  : _started(false), _joined(false),  _func(func), _name(name)
{
  setDefaultName();
}

Thread::Thread(ThreadFunc &&func, const std::string &name)
  : _started(false), _joined(false), _func(std::move(func)), _name(name)
{
  setDefaultName();
}

Thread::~Thread() {
  if (_started && !_joined) {
    pthread_detach(_threadId);
  }
}

void Thread::setDefaultName() {
  std::atomic_fetch_add(&_numCreated, 1);
  int num = numCreated();
  if (_name.empty()) {
    char buf[32];
    snprintf(buf, sizeof(buf), "Thread%d", num);
    _name = buf;
  }
}

void Thread::start() {
  assert(!_started);
  _started = true;
  auto data = new detail::ThreadData(_func, _name, _tid);
  if (::pthread_create(&_threadId, nullptr, &detail::startThread, data)) {
    _started = false;
    LOG_SYSFATAL << "Failed in pthread_create";
  }
}

int Thread::join() {
  assert(_started);
  assert(!_joined);
  _joined = true;
  return ::pthread_join(_threadId, nullptr);

}

}  // namespace tnet
