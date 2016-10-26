#include <tnet/net/Acceptor.h>

#include <tnet/base/Logging.h>
#include <tnet/net/EventLoop.h>
#include <tnet/net/InetAddress.h>
#include <tnet/net/SocketsOps.h>

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

using namespace tnet;
using namespace tnet::net;

Acceptor::Acceptor(EventLoop* loop,
                   const InetAddress& listenAddr,
                   bool reusePort)
    : _loop(loop),
    _acceptSocket(sockets::createNonblockingOrDie(listenAddr.family())),
    _acceptChannel(_loop, _acceptSocket.fd()),
    _listenning(false),
    _idleFd(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
  assert(_idleFd > 0);
  _acceptSocket.setReuseAddr(true);
  _acceptSocket.setReusePort(reusePort);
  _acceptSocket.bindAddress(listenAddr);
  _acceptChannel.setReadCallback([this]{ handleRead(); });
}

Acceptor::~Acceptor() {
  _acceptChannel.disableAll();
  _acceptChannel.remove();
  ::close(_idleFd);
}

void Acceptor::listen() {
  _loop->assertInLoopThread();
  _listenning = true;
  _acceptSocket.listen();
  _acceptChannel.enableReading();
}

void Acceptor::handleRead() {
  _loop->assertInLoopThread();
  InetAddress peerAddr;
  int connfd = _acceptSocket.accept(&peerAddr);
  if (connfd > 0) {
    if (_newConnectionCallback) {
      _newConnectionCallback(connfd, peerAddr);
    } else {
      sockets::close(connfd);
    }
  } else {
    LOG_SYSERR << "in Acceptor::handleRead";
    if (errno == EMFILE) {
      ::close(_idleFd);
      _idleFd = ::accept(_acceptSocket.fd(), nullptr, nullptr);
      ::close(_idleFd);
      _idleFd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
    }
  }
}
