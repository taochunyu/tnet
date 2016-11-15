#include <tnet/net/TcpClient.h>
#include <tnet/base/Logging.h>
#include <tnet/net/EventLoop.h>
#include <tnet/net/TimerId.h>
#include <functional>

using namespace tnet;
using namespace tnet::net;

auto handleConn = [](auto conn) {
  if (conn->connected()) {
    conn->send(" world");
  }
};

auto handleMess = [](auto conn, auto buf, auto time) {
  std::string msg(buf->retrieveAllAsString());
  LOG_INFO << conn->name() << " recv " << msg << " bytes at " << time.toString();
  conn->send(msg);
};

int main(int argc, char const *argv[]) {
  EventLoop loop;
  InetAddress server("127.0.0.1", 8080);
  TcpClient client(&loop, server, "TcpClient");
  client.onConnection(handleConn);
  client.onMessage(handleMess);
  client.connect();
  loop.loop();
  return 0;
}
