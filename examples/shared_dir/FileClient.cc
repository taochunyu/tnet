#include "FileClient.h"

FileClient::FileClient(EventLoop* loop, InetAddress serverAddr, FileModelClient& fmc)
    : _loop(loop),
      _client(_loop, serverAddr, "FileClient"),
      _fmc(fmc)
{
  _client.onConnection([this](auto conn){ handleConn(conn); });
  _client.onWriteCompleted([this](auto conn){ whenWriteComplete(conn); });
  _client.onMessage([this](auto conn, auto buf, auto receiveTime){
    whenReceiveData(conn, buf, receiveTime);
  });
}

void FileClient::handleConn(const TcpConnectionPtr& conn) {
  LOG_INFO
    << conn->peerAddress().toIpPort() << " -> "
    << conn->localAddress().toIpPort() << " is "
    << (conn->connected() ? "UP" : "DOWN");
  {
    MutexLockGuard lck(_mtx);
    if (conn->connected()) {
      _conn = conn;
    } else {
      _conn.reset();
    }
  }
}
