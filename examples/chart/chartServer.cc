#include <tnet/net/TcpServer.h>
#include <tnet/base/nocopyable.h>
#include <tnet/base/Logging.h>
#include <tnet/net/Buffer.h>
#include <tnet/net/EventLoop.h>
#include <tnet/net/InetAddress.h>
#include <tnet/net/TcpConnection.h>
#include <tnet/net/TimerId.h>
#include <tnet/net/Codec.h>
#include <tnet/net/Dispather.h>
#include <set>
#include <stdio.h>

using namespace tnet;
using namespace tnet::net;

class ChartServer : tnet::nocopyable {
 public:
  ChartServer(EventLoop* loop, const InetAddress& listenAddr);
  void start() { _server.start(); }
 private:
  void handleConn(const TcpConnectionPtr& conn);
  void handleMess(const TcpConnectionPtr& conn,
                  std::string method,
                  std::string message,
                  Timestamp receiveTime);
  EventLoop* _loop;
  TcpServer _server;
  Codec _codec;
  std::set<TcpConnectionPtr> _connList;
};

ChartServer::ChartServer(EventLoop* loop, const InetAddress& listenAddr)
    : _loop(loop),
      _server(loop, listenAddr, "ChartServer"),
      _codec([this](auto a, auto b, auto c, auto d){ handleMess(a, b, c, d); }) {
  _server.onConnection([this](auto conn) {
    handleConn(conn);
  });
  _server.onMessage([this](auto conn, auto buf, auto receiveTime) {
    _codec.onMessage(conn, buf, receiveTime);
  });
  (void)_loop;
}

void ChartServer::handleConn(const TcpConnectionPtr& conn) {
  LOG_INFO
    << conn->peerAddress().toIpPort() << " -> "
    << conn->localAddress().toIpPort() << " is "
    << (conn->connected() ? "UP" : "DOWN");
  if (conn->connected()) {
    _connList.insert(conn);
  }
  if (conn->disconnected()) {
    _connList.erase(conn);
  }
}

void ChartServer::handleMess(const TcpConnectionPtr& conn,
                std::string method,
                std::string message,
                Timestamp receiveTime) {
  for (auto it = _connList.begin(); it != _connList.end(); it++) {
    _codec.send(*it, "broadcast", message);
  }
}

int main(int argc, char const *argv[]) {
  EventLoop loop;
  InetAddress serverAddr(8080);
  ChartServer server(&loop, serverAddr);
  server.start();
  loop.loop();
  return 0;
}
