#ifndef TNET_TBASE_CURRENT_THREAD_H
#define TNET_TBASE_CURRENT_THREAD_H

#include <stdint.h>

namespace tnet {
namespace CurrentThread {

extern thread_local int t_cacheTid;
extern thread_local char t_tidString[32];
extern thread_local int t_tidStringLength;
extern thread_local const char* t_threadName;
void cacheTid();

inline int tid() {
  // __builtin_expect(a, b) 分支预测 a 为 b 的概率极高
  if (__builtin_expect(t_cacheTid == 0, 0)) {
    cacheTid();
  }
  return t_cacheTid;
}

inline const char* tidString() {
  return t_tidString;
}

inline int tidStringLength() {
  return t_tidStringLength;
}

inline const char* name() {
  return t_threadName;
}

bool isMainThread();
void sleepUsec(int64_t usec);

}  // namespace CurrentThread
}  // namespace tnet

#endif  // TNET_TBASE_CURRENT_THREAD_H
