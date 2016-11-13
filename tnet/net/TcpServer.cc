#include <tnet/net/TcpServer.h>
#include <tnet/base/Logging.h>
#include <tnet/net/Acceptor.h>
#include <tnet/net/InetAddress.h>
#include <tnet/net/EventLoop.h>
#include <tnet/net/EventLoopThreadPool.h>
#include <tnet/net/SocketsOps.h>
#include <tnet/net/TcpConnection.h>
#include <memory>
#include <string>
#include <stdio.h>

using namespace tnet;
using namespace tnet::net;

TcpServer::TcpServer(EventLoop* loop,
                     const InetAddress& listenAddr,
                     const std::string& nameArg,
                     Option option)
    : _loop(loop),
      _ipPort(listenAddr.toIpPort()),
      _name(nameArg),
      _acceptor(new Acceptor(loop, listenAddr, option == kReusePort)),
      _threadPool(new EventLoopThreadPool(_loop, _name)),
      _connectionCallback(defaultConnectionCallback),
      _messageCallback(defaultMessageCallback),
      _nextConnId(1) {
  _acceptor->setNewConnectionCallback([this](auto fd, auto peerAddr) {
    newConnection(fd, peerAddr);
  });
}

TcpServer::~TcpServer() {
  _loop->assertInLoopThread();
  LOG_TRACE << "TcpServer::~TcpServer[" << _name << "] destructing";
  for (auto it = _connections.begin(); it != _connections.end(); it++) {
    std::shared_ptr<TcpConnection> conn(it->second);
    it->second.reset();
    conn->getLoop()->runInLoop([&conn]{ conn->destoryConnection(); });
    conn.reset();
  }
}

void TcpServer::setThreadNum(int numThreads) {
  assert(numThreads >= 0);
  _threadPool->setThreadNum(numThreads);
}

void TcpServer::start() {
  if (_started.load() == 0) {
    _started.store(1);
    _threadPool->start(_threadInitCallback);
    assert(!_acceptor->listenning());
    _loop->runInLoop([this]{
      _acceptor->listen();
      LOG_INFO << "TcpServer started";
    });
  }
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
  _loop->assertInLoopThread();
  EventLoop* ioLoop = _threadPool->getNextLoop();
  char buf[64];
  snprintf(buf, sizeof(buf), "-%s#%d", _ipPort.c_str(), _nextConnId);
  ++_nextConnId;
  std::string connName = _name + buf;
  LOG_INFO
    << "TcpConnection::newConnection [" << _name
    << "] - new connection [" << connName
    << "] from " << peerAddr.toIpPort();
  InetAddress localAddr(sockets::getLocalAddr(sockfd));
  auto conn = std::make_shared<TcpConnection>(ioLoop, connName, sockfd, localAddr, peerAddr);
  _connections[connName] = conn;
  conn->onConnected(_connectionCallback);
  conn->onMessage(_messageCallback);
  conn->onWriteCompleted(_writeCompletedCallback);
  conn->onClose([this](auto conn){ removeConnection(conn); });
  // 下一个事件循环建立
  ioLoop->runInLoop([&conn]{ conn->establishConnection(); });
}

void TcpServer::removeConnection(const std::shared_ptr<TcpConnection>& conn) {
  _loop->runInLoop([this, conn]{ removeConnectionInLoop(conn); });
}

void TcpServer::removeConnectionInLoop(const std::shared_ptr<TcpConnection>& conn) {
  _loop->assertInLoopThread();
  LOG_INFO
    << "TcpServer::removeConnectionInLoop [" << _name
    << "] - connection " << conn->name();
  size_t n = _connections.erase(conn->name());
  assert(n == 1);
  EventLoop* ioLoop = conn->getLoop();
  ioLoop->queueInLoop([conn]{
    conn->destoryConnection();
  });
}
