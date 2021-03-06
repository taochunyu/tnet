#include <tnet/net/TcpServer.h>
#include <tnet/base/nocopyable.h>
#include <tnet/base/Logging.h>
#include <tnet/net/Buffer.h>
#include <tnet/net/EventLoop.h>
#include <tnet/net/InetAddress.h>
#include <tnet/net/TcpConnection.h>
#include <tnet/net/TimerId.h>
#include <tnet/net/Codec.h>
#include <tnet/base/Mutex.h>
#include <set>
#include <stdio.h>

using namespace tnet;
using namespace tnet::net;

class ChatServer : tnet::nocopyable {
 public:
  ChatServer(EventLoop* loop, const InetAddress& listenAddr);
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
  MutexLock _mtx;
};

ChatServer::ChatServer(EventLoop* loop, const InetAddress& listenAddr)
    : _loop(loop),
      _server(_loop, listenAddr, "ChatServer"),
      _codec([this](auto a, auto b, auto c, auto d){ handleMess(a, b, c, d); }) {
  _server.onConnection([this](auto conn) {
    handleConn(conn);
  });
  _server.onMessage([this](auto conn, auto buf, auto receiveTime) {
    _codec.onMessage(conn, buf, receiveTime);
  });
}

void ChatServer::handleConn(const TcpConnectionPtr& conn) {
  LOG_INFO
    << conn->peerAddress().toIpPort() << " -> "
    << conn->localAddress().toIpPort() << " is "
    << (conn->connected() ? "UP" : "DOWN");
  if (conn->connected()) {
    MutexLockGuard lck(_mtx);
    _connList.insert(conn);
  }
  if (conn->disconnected()) {
    MutexLockGuard lck(_mtx);
    _connList.erase(conn);
  }
}

void ChatServer::handleMess(const TcpConnectionPtr& conn,
                std::string method,
                std::string message,
                Timestamp receiveTime) {
  MutexLockGuard lck(_mtx);
  for (auto it = _connList.begin(); it != _connList.end(); it++) {
    _codec.send(*it, method, message);
  }
}

int main(int argc, char const *argv[]) {
  EventLoop loop;
  InetAddress serverAddr(8080);
  ChatServer server(&loop, serverAddr);
  server.start();
  loop.loop();
  return 0;
}
