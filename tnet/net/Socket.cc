#include <tnet/net/Socket.h>
#include <tnet/base/Logging.h>
#include <tnet/net/InetAddress.h>
#include <tnet/net/SocketsOps.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <stdio.h>

using namespace tnet;
using namespace tnet::net;

Socket::~Socket() {
  sockets::close(_sockfd);
}

bool Socket::getTcpInfo(struct tcp_info* tcpi) const {
  return false;
}

bool Socket::getTcpInfoString(char* buf, int len) const {
  return false;
}

void Socket::bindAddress(const InetAddress& addr) {
  sockets::bindOrDie(_sockfd, addr.getSockAddr());
}

void Socket::listen() {
  sockets::listenOrDie(_sockfd);
}

int Socket::accept(InetAddress* peerAddr) {
  struct sockaddr_in6 addr;
  bzero(&addr, sizeof(addr));
  int connfd = sockets::accept(_sockfd, &addr);
  if (connfd >= 0) {
    peerAddr->setSockAddr6(addr);
  }
  return connfd;
}

void Socket::shutdownWrite() {
  sockets::shutdownWrite(_sockfd);
}

void Socket::setTcpNoDelay(bool on) {
  int optval = on ? 1 : 0;
  int ret = ::setsockopt(_sockfd, IPPROTO_TCP, TCP_NODELAY,
               &optval, static_cast<socklen_t>(sizeof(optval)));

  if (ret < 0 && on) {
    LOG_SYSERR << "Socket::setTcpNoDely";
  }
}

void Socket::setReuseAddr(bool on) {
  int optval = on ? 1 : 0;
  int ret = ::setsockopt(_sockfd, IPPROTO_TCP, SO_REUSEADDR,
               &optval, static_cast<socklen_t>(sizeof(optval)));
  if (ret < 0 && on) {
    LOG_SYSERR << "Socket::setReuseAddr";
  }
}

void Socket::setReusePort(bool on) {
  int optval = on ? 1 : 0;
  int ret = ::setsockopt(_sockfd, IPPROTO_TCP, SO_REUSEADDR,
               &optval, static_cast<socklen_t>(sizeof(optval)));
  if (ret < 0 && on) {
    LOG_SYSERR << "Socket::setReusePort";
  }
}

void Socket::setKeepAlive(bool on) {
  int optval = on ? 1 : 0;
  int ret = ::setsockopt(_sockfd, IPPROTO_TCP, SO_KEEPALIVE,
               &optval, static_cast<socklen_t>(sizeof(optval)));
  if (ret < 0 && on) {
    LOG_SYSERR << "Socket::setReusePort";
  }
}
