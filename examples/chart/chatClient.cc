#include <tnet/net/TcpClient.h>
#include <tnet/base/nocopyable.h>
#include <tnet/base/Logging.h>
#include <tnet/net/Buffer.cc>
#include <tnet/net/EventLoop.h>
#include <tnet/net/InetAddress.h>
#include <tnet/net/TcpConnection.h>
#include <tnet/net/EventLoopThread.h>
#include <tnet/net/Codec.h>
#include <tnet/base/Mutex.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>

class ChatClient : tnet::nocopyable {
 public:
  ChatClient(EventLoop* loop, const InetAddress& serverAddr)
      : _loop(loop),
        _client(_loop, serverAddr, "ChatClient"),
        _codec([this](auto a, auto b, auto c, auto d){ handleMess(a, b, c, d); }) {
    _client.onConnection([this](auto conn) {
      handleConn(conn);
    });
    _client.onMessage([this](auto conn, auto buf, auto receiveTime) {
      _codec.onMessage(conn, buf, receiveTime);
    });
    char buf[64];
    sprintf(buf, "%d", (int)::getpid());
    _name = buf;
  }
  void connect() {
    _client.connect();
  }
  void write(const std::string& message) {
    MutexLockGuard lck(_mtx);
    if (_connection) {
      _codec.send(_connection, _name, message);
    }
  }
 private:
  void handleConn(const TcpConnectionPtr& conn) {
    LOG_INFO
      << conn->peerAddress().toIpPort() << " -> "
      << conn->localAddress().toIpPort() << " is "
      << (conn->connected() ? "UP" : "DOWN");
    MutexLockGuard lck(_mtx);
    if (conn->connected()) {
      _connection = conn;
    } else {
      _connection.reset();
    }
  }
  void handleMess(const TcpConnectionPtr& conn,
                  const std::string method,
                  const std::string message,
                  Timestamp receiveTime) {
    printf("\n>>用户 %s 说:\n>> %s\n\n", method.c_str(), message.c_str());
  }
  EventLoop* _loop;
  TcpClient _client;
  Codec _codec;
  MutexLock _mtx;
  TcpConnectionPtr _connection;
  std::string _name;
};

int main(int argc, char const *argv[]) {
  EventLoopThread loopThread;
  InetAddress serverAddr("127.0.0.1", 8080);
  ChatClient client(loopThread.startLoop(), serverAddr);
  client.connect();
  std::string line;
  while (std::getline(std::cin, line)) {
    client.write(line);
  }
  return 0;
}
