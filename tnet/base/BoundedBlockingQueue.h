#ifndef TNET_BASE_BOUNDEDBLOCKINGQUEUE_H
#define TNET_BASE_BOUNDEDBLOCKINGQUEUE_H

#include "BoundedQueue.h"
#include <mutex>
#include <condition_variable>
#include <assert.h>

template<typename T>
class BoundedBlockingQueue {
 public:
  explicit BoundedBlockingQueue(std::size_t capacity)
  : _queue(capacity) {}
  std::size_t size() {
    return _queue.size();
  }
  void put(const T &item) {
    std::unique_lock<std::mutex> lck(_mutex);
    while (_queue.full()) {
      _not_full.wait(lck);
    }
    assert(!_queue.full());
    _queue.push_back(item);
    _not_empty.notify_one();
  }
  T take() {
    std::unique_lock<std::mutex> lck(_mutex);
    while (_queue.empty()) {
      _not_empty.wait(lck);
    }
    assert(!_queue.empty());
    auto front = _queue.front();
    _queue.pop_front();
    _not_full.notify_one();
    return front;
  }
 private:
  std::mutex _mutex;
  std::condition_variable _not_empty;
  std::condition_variable _not_full;
  BoundedQueue<T> _queue;
};

#endif  // TNET_BASE_BOUNDEDBLOCKINGQUEUE_H
