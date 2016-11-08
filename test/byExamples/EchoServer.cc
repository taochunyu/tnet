//test TcpServer and TcpConnection

#include <tnet/net/TcpServer.h>
#include <tnet/base/Logging.h>
#include <tnet/net/Buffer.cc>
#include <tnet/net/EventLoop.h>
#include <tnet/net/InetAddress.h>
#include <tnet/net/TcpConnection.h>
#include <stdio.h>

using namespace tnet;
using namespace tnet::net;

auto handleConn = [](auto conn) {
  printf("hello\n");
  conn->send("hello");
};

auto handleMess = [](auto conn, auto buf, auto time) {
  std::string msg(buf->retrieveAllAsString());
  LOG_TRACE << conn->name() << " recv " << msg.size() << " bytes at " << time.toString();
  conn->send(msg);
};

int main(int argc, char const *argv[]) {
  EventLoop loop;
  InetAddress listenAddr(2000);
  TcpServer server(&loop, listenAddr, "EchoSever");
  server.onConnection(handleConn);
  server.onMessage(handleMess);
  server.setThreadNum(1);
  server.start();
  loop.loop();
  return 0;
}