//test TcpServer and TcpConnection

#include <tnet/net/TcpServer.h>
#include <tnet/base/Logging.h>
#include <tnet/net/Buffer.cc>
#include <tnet/net/EventLoop.h>
#include <tnet/net/InetAddress.h>
#include <tnet/net/TcpConnection.h>
#include <tnet/net/TimerId.h>
#include <stdio.h>

using namespace tnet;
using namespace tnet::net;

auto handleConn = [](auto conn) {
  if (conn->connected()) {
    //auto id = conn->getLoop()->runEvery(2, []{ printf("新连接说每两秒要显示一次这句话\n"); });
    //conn->ctx(id);
    conn->send("hello");
  }
  if (conn->disconnected()) {
    //conn->getLoop()->cancel(any_cast<TimerId>(conn->ctx()));
    //printf("新连接断开，不再说话了\n");
  }
};

auto handleMess = [](auto conn, auto buf, auto time) {
  std::string msg(buf->retrieveAllAsString());
  LOG_INFO << conn->name() << " recv " << msg.size() << " bytes at " << time.toString();
  conn->send(msg);
};

int main(int argc, char const *argv[]) {
  EventLoop loop;
  InetAddress listenAddr(8080, true, false);
  TcpServer server(&loop, listenAddr, "EchoSever");
  server.onConnection(handleConn);
  server.onMessage(handleMess);
  server.setThreadNum(1);
  server.start();
  LOG_INFO << "EchoSever Start main loop is " << &loop;
  //loop.runEvery(1, []{ printf("主循环要一秒强调一下自己\n"); });
  loop.loop();
  return 0;
}
