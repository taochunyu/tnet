#include <tnet/base/ThreadPool.h>
#include <tnet/base/Exception.h>
#include <assert.h>
#include <stdio.h>
#include <functional>
#include <string>
#include <memory>

namespace tnet {

ThreadPool::ThreadPool(const std::string &name)
  : _mtx(),
    _notEmpty(_mtx),
    _notFull(_mtx),
    _name(name),
    _maxQueueSize(0),
    _running(false) {}
ThreadPool::~ThreadPool() {
  if (_running) {
    stop();
  }
}

void ThreadPool::start(int numThreads) {
  assert(_threads.empty());
  _running = true;
  for (int i = 0; i < numThreads; i++) {
    char id[32];
    snprintf(id, sizeof id, "%d", i + 1);
    _threads.push_back(std::make_shared<Thread>([this]{ runInThread(); }, _name + id));
    _threads[i] -> start();
  }
  if (numThreads == 0 && _threadInitCallback) {
    _threadInitCallback();
  }
}

void ThreadPool::stop() {
  MutexLockGuard lck(_mtx);
  printf("stop\n");
  _running = false;
  if (!_running) {
    printf("unrunning\n");
  }
  _notEmpty.notifyAll();
  for_each(_threads.begin(), _threads.end(),
    [](std::shared_ptr<Thread> p) { p -> join(); });
}

std::size_t ThreadPool::queueSize() const {
  MutexLockGuard lck(_mtx);
  return _queue.size();
}

void ThreadPool::run(const Task &task) {
  if (_threads.empty()) {
    task();
  } else {
    MutexLockGuard lck(_mtx);
    while (isFull()) {
      _notFull.wait();
    }
    assert(!isFull());
    _queue.push_back(task);
    _notEmpty.notify();
  }
}

void ThreadPool::run(Task &&task) {
  if (_threads.empty()) {
    task();
  } else {
    MutexLockGuard lck(_mtx);
    while (isFull()) {
      _notFull.wait();
    }
    assert(!isFull());
    _queue.push_back(std::move(task));
    _notEmpty.notify();
  }
}

ThreadPool::Task ThreadPool::take() {
  MutexLockGuard lck(_mtx);
  // always use a while-loop, due to spurious wakeup
  printf("%d is taking\n", CurrentThread::tid());
  while (_queue.empty() && _running) {
    printf("%d will wait\n", CurrentThread::tid());
    _notEmpty.wait();
    if (!_running) {
      printf("unrunning\n");
    }
  }
  Task task;
  if (!_queue.empty()) {
    task = _queue.front();
    _queue.pop_front();
    if (_maxQueueSize > 0) {
      _notFull.notify();
    }
  }
  if (!_running) {
    printf("notask\n");
  }
  return task;
}

bool ThreadPool::isFull() const {
  _mtx.assertLocked();
  return _maxQueueSize > 0 && _queue.size() >= _maxQueueSize;
}

void ThreadPool::runInThread() {
  try {
    if (_threadInitCallback) {
      _threadInitCallback();
    }
    while (_running) {
      printf("%d is running\n", CurrentThread::tid());
      printf("hi1\n");
      Task task(take());
      printf("hi2\n");
      if (task) {
        printf("task\n");
        task();
      }
    }
    if (!_running) {
      printf("unrunning\n");
    }
  } catch (const Exception &ex) {
    fprintf(stderr, "Exception caught in ThreadPool %s\n", _name.c_str());
    fprintf(stderr, "Reason: %s\n", ex.what());
    fprintf(stderr, "Stack trace: %s\n", ex.stackTrace());
    abort();
  } catch (const std::exception &ex) {
    fprintf(stderr, "Exception caught in ThreadPool %s\n", _name.c_str());
    fprintf(stderr, "Reason: %s\n", ex.what());
    abort();
  } catch (...) {
    fprintf(stderr, "Unknown exception caught in ThreadPool %s\n", _name.c_str());
    throw;
  }
}

}  // namespace tnet
