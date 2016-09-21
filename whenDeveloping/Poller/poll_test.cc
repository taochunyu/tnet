#include <tnet/net/EventLoop.h>
#include <tnet/net/Channel.h>
#include <tnet/base/TimerFd.cc>
#include <tnet/base/Timestamp.h>

using namespace tnet;
using namespace net;


void timeout() {
  printf("Timeout!\n");
}

int main() {
  tnet::net::EventLoop loop;

  Timestamp now = Timestamp::now();
  int64_t absMs = now.microSecondsSinceEpoch() + 5000;

  int fd = timerfd::timerfd.create();
  timerfd::timerfd.setTime(fd, absMs);

  tnet::net::Channel channel(&loop, fd);
  channel.setReadCallback(timeout);
  channel.enableReading();

  loop.loop();

  return 0;
}
