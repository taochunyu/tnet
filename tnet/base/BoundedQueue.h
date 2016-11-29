#ifndef TNET_BASE_BOUNDEDQUEUE_H
#define TNET_BASE_BOUNDEDQUEUE_H

#include <deque>

template<typename T>
class BoundedQueue {
 public:
   BoundedQueue(std::size_t capacity = 1000)
   : _queue(), _capacity(capacity) {}
   bool full() const { return _queue.size() >= _capacity; }
   bool empty() const { return _queue.empty(); }
   std::size_t size() const { return _queue.size(); }
   void push_back(const T &item) {
     if (full()) return;
     _queue.push_back(item);
   }
   void pop_front() {
     if (empty()) return;
     _queue.pop_front();
   }
   T& front() {
     return _queue.front();
   }
   T& operator[](std::size_t i) {
     return _queue[i];
   }
 private:
  std::deque<T> _queue;
  std::size_t _capacity;
};

#endif  // TNET_BASE_BOUNDEDQUEUE_H
