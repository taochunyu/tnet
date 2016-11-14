#include <tnet/net/TcpClient.h>
#include <tnet/base/Thread.h>
#include <tnet/base/Logging.h>
#include <tnet/net/EventLoop.h>
#include <tnet/net/TimerId.h>
#include <functional>

using namespace tnet;
using namespace tnet::net;

void threadFunc(EventLoop* loop) {
  InetAddress serverAddr("127.0.0.1", 8080);
  TcpClient client(loop, serverAddr, "TcpClient");
  client.connect();

  CurrentThread::sleepUsec(1000 * 1000);
}

int main(int argc, char const *argv[]) {
  EventLoop loop;
  loop.runAfter(3.0, [&loop]{ loop.quit(); });
  Thread thr([&loop]{ threadFunc(&loop); });
  thr.start();
  loop.loop();
  return 0;
}
