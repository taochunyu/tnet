#include <tnet/net/EventLoop.h>
#include <tnet/net/Channel.h>
#include <tnet/base/TimerFd.h>
#include <tnet/base/Timestamp.h>

using namespace tnet;
using namespace net;
using namespace timerfd;


void timeout() {
  printf("Timeout!\n");
}

int main() {
  tnet::net::EventLoop loop;

  Timestamp now = Timestamp::now();
  Timestamp expiration(now.microSecondsSinceEpoch() + 2000);

  int fd = createTimerfd();
  resetTimerFd(fd, expiration);

  tnet::net::Channel channel(&loop, fd);
  channel.setReadCallback(timeout);
  channel.enableReading();

  loop.loop();

  return 0;
}
