#include <tnet/net/EventLoop.h>
#include <tnet/net/Channel.h>
#include <tnet/base/TimerFd.h>
#include <stdio.h>
#include <chrono>

tnet::net::EventLoop* g_loop;

void timeout() {
  printf("Timeout!\n");
}

int main() {
  tnet::net::EventLoop loop;
  g_loop = &loop;

  tnet::TimerFd timerfd;

  timerfd.setTime(std::chrono::milliseconds(5000));
  tnet::net::Channel channel(&loop, timerfd.getFd());

  channel.setReadCallback(timeout);
  channel.enableReading();

  return 0;
}
