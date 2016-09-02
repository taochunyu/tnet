#include <tnet/net/EventLoop.h>
#include <tnet/base/CurrentThread.h>
#include <tnet/base/Thread.h>
#include <unistd.h>
#include <stdio.h>
void threadFunc() {
  printf("threadFunc(): pid = %d, tid = %d\n", ::getpid(), tnet::CurrentThread::tid());
  tnet::net::EventLoop loop;
  loop.loop();
}

int main() {
  printf("main(): pid = %d, tid = %d\n", ::getpid(), tnet::CurrentThread::tid());
  tnet::net::EventLoop loop;
  tnet::Thread thread(threadFunc);
  thread.start();
  loop.loop();
  pthread_exit(nullptr);
  return 0;
}
