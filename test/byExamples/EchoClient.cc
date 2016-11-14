#include <tnet/net/TcpClient.h>
#include <tnet/base/Logging.h>
#include <tnet/net/EventLoop.h>
#include <tnet/net/TimerId.h>
#include <functional>

using namespace tnet;
using namespace tnet::net;

int main(int argc, char const *argv[]) {
  EventLoop loop;
  InetAddress server("127.0.0.1", 8080);
  TcpClient client(&loop, server, "TcpClient");
  loop.runAfter(0.0, [&client]{
    LOG_INFO << "timeout";
    client.stop();
  });
  loop.runAfter(1.0, [&loop]{ loop.quit(); });
  client.connect();
  CurrentThread::sleepUsec(100 * 1000);
  loop.loop();
  return 0;
}
