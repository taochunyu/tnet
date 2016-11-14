#include <tnet/net/TcpClient.h>
#include <tnet/base/Logging.h>
#include <tnet/net/Connector.h>
#include <tnet/net/EventLoop.h>
#include <tnet/net/SocketsOps.h>
#include <functional>

#include <stdio.h>

using namespace tnet;
using namespace tnet::net;

TcpClient::TcpClient(EventLoop* loop,
                     const InetAddress& serverAddr,
                     const std::string& nameArg)
    : _loop(loop),
      _connector(new Connector(loop, serverAddr)),
      _name(nameArg),
      _connectionCallback(defaultConnectionCallback),
      _messageCallback(defaultMessageCallback),
      _retry(false),
      _connect(true),
      _nextConnId(1) {
  _connector->setNewConnectionCallback([this](auto conn) {
    newConnection(conn);
  });
  LOG_INFO
    << "TcpClient::TcpClient[" << _name
    << "] - connector " << _connector.get();
}

TcpClient::~TcpClient() {
  LOG_INFO
    << "TcpClient::~TcpClient[" << _name
    << "] - connector " << _connector.get();
  TcpConnectionPtr conn;
  bool unique = false;
  {
    MutexLockGuard lock(_mutex);
    unique = _connection.unique();
    conn = _connection;
  }
  if (conn) {
    assert(_loop == conn->getLoop());
    _loop->runInLoop([conn, this] {
      conn->onClose([this](auto conn){
        _loop->queueInLoop([conn]{
          conn->destoryConnection();
        });
      });
    });
    if (unique) {
      conn->forceClose();
    }
  } else {
    _connector->stop();
  }
}

void TcpClient::connect() {
  LOG_INFO
    << "TcpClient::connect[" << _name << "] - connecting to "
    << _connector->serverAddress().toIpPort();
  _connect = true;
  _connector->start();
}

void TcpClient::disconnect() {
  {
    MutexLockGuard lock(_mutex);
    if (_connect) {
      _connection->shutdown();
    }
  }
}

void TcpClient::stop() {
  _connect = false;
  _connector->stop();
}

void TcpClient::newConnection(int sockfd) {
  _loop->assertInLoopThread();
  InetAddress peerAddr(sockets::getPeerAddr(sockfd));
  char buf[32];
  snprintf(buf, sizeof(buf), ":%s#%d", peerAddr.toIpPort().c_str(), _nextConnId);
  ++_nextConnId;
  std::string connName = _name + buf;
  InetAddress localAddr(sockets::getLocalAddr(sockfd));
  auto conn = std::make_shared<TcpConnection>(_loop, connName, sockfd, localAddr, peerAddr);
  conn->onConnected(_connectionCallback);
  conn->onMessage(_messageCallback);
  conn->onWriteCompleted(_writeCompletedCallback);
  conn->onClose([this](auto conn){ removeConnection(conn); });
  {
    MutexLockGuard lock(_mutex);
    _connection = conn;
  }
  conn->establishConnection();
}

void TcpClient::removeConnection(const std::shared_ptr<TcpConnection>& conn)
{
  _loop->assertInLoopThread();
  assert(_loop == conn->getLoop());
  {
    MutexLockGuard lock(_mutex);
    assert(_connection == conn);
    _connection.reset();
  }
  _loop->queueInLoop([conn]{
    conn->destoryConnection();
  });
  if (_retry && _connect) {
    LOG_INFO
      << "TcpClient::connect[" << _name << "] - Reconnecting to "
      << _connector->serverAddress().toIpPort();
    _connector->restart();
  }
}
