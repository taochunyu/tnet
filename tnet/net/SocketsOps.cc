#include <tnet/net/SocketsOps.h>
#include <tnet/base/Logging.h>
#include <tnet/base/Types.h>
#include <tnet/net/Endian.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/uio.h>
#include <assert.h>

namespace tnet {
namespace net {
namespace sockets {

namespace {

void setNonblockingAndCloseExec(int sockfd) {
  int flags = ::fcntl(sockfd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  int ret = ::fcntl(sockfd, F_SETFL, flags);
  if (ret < 0) {
    LOG_SYSFATAL << "sockets::setNonblockingAndCloseExec: " << strerror(errno);
  }
  flags =::fcntl(sockfd, F_GETFD, 0);
  flags |= FD_CLOEXEC;
  ret = ::fcntl(sockfd, F_SETFD, flags);
  if (ret < 0) {
    LOG_SYSFATAL << "sockets::setNonblockingAndCloseExec: " << strerror(errno);
  }
}

}

const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr) {
  return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}

struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr) {
  return static_cast<struct sockaddr*>(implicit_cast<void*>(addr));
}

const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr) {
  return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}

const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr) {
  return static_cast<const struct sockaddr_in*>(implicit_cast<const void*>(addr));
}

const struct sockaddr_in6* sockaddr_in6_cast(const struct sockaddr* addr) {
  return static_cast<const struct sockaddr_in6*>(implicit_cast<const void*>(addr));
}

int createNonblockingOrDie(sa_family_t family) {
  int sockfd = ::socket(family, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd < 0) {
    LOG_SYSFATAL << "sockets::createNonblockingOrDie: " << strerror(errno);
  }
  setNonblockingAndCloseExec(sockfd);
  return sockfd;
}

void bindOrDie(int sockfd, const struct sockaddr* addr) {
  int ret = ::bind(sockfd, addr, addr->sa_len);
  if (ret < 0) {
    LOG_SYSFATAL << "sockets::bindOrDie: " << strerror(errno);
  }
}

void listenOrDie(int sockfd) {
  int ret = ::listen(sockfd, SOMAXCONN);
  if (ret < 0) {
    LOG_SYSFATAL << "sockets::listenOrDie: " << strerror(errno);
  }
}

int accept(int sockfd, struct sockaddr_in6* addr) {
  socklen_t addrlen = static_cast<socklen_t>(sizeof(struct sockaddr_in6));
  int connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
  if (connfd < 0) {
    int savedErrno = errno;
    LOG_SYSERR << "socket::accept";
    switch (savedErrno) {
      case EAGAIN:
      case ECONNABORTED:
      case EINTR:
      case EPROTO:
      case EPERM:
      case EMFILE:
        errno = savedErrno;
        break;
      case EBADF:
      case EFAULT:
      case EINVAL:
      case ENFILE:
      case ENOBUFS:
      case ENOMEM:
      case ENOTSOCK:
      case EOPNOTSUPP:
        LOG_FATAL << "unexpected error of ::accept " << savedErrno;
        break;
      default:
        LOG_FATAL << "unknown error of ::accept";
    }
  }
  return connfd;
}

int connect(int sockfd, const struct sockaddr* addr) {
  return ::connect(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
}

ssize_t read(int sockfd, void* buf, size_t count) {
  return ::read(sockfd, buf, count);
}

ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt) {
  return ::readv(sockfd, iov, iovcnt);
}

ssize_t write(int sockfd, const void* buf, size_t count) {
  return ::write(sockfd, buf, count);
}

void close(int sockfd) {
  if (::close(sockfd) < 0) {
    LOG_SYSERR << "sockets::close";
  }
}

void shutdownWrite(int sockfd) {
  if (::shutdown(sockfd, SHUT_WR) < 0) {
    LOG_SYSERR << "sockets::shutdownWrite";
  }
}

void toIpPort(char* buf, size_t size, const struct sockaddr* addr) {
  toIp(buf, size, addr);
  size_t end = ::strlen(buf);
  const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
  uint16_t port = sockets::networkToHost16(addr4->sin_port);
  assert(size > end);
  snprintf(buf + end, size - end, ":%u", port);
}

void toIp(char* buf, size_t size, const struct sockaddr* addr) {
  if (addr->sa_family == AF_INET) {
    assert(size >= INET_ADDRSTRLEN);
    const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
    ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
  }
  if (addr->sa_family == AF_INET6) {
    assert(size >= INET6_ADDRSTRLEN);
    const struct sockaddr_in6* addr6 = sockaddr_in6_cast(addr);
    ::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(size));
  }
}

void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr) {
  addr->sin_family = AF_INET;
  addr->sin_port = hostToNetwork16(port);
  if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0) {
    LOG_SYSERR << "sockets::fromIpPort";
  }
}

void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr) {
  addr->sin6_family = AF_INET6;
  addr->sin6_port = hostToNetwork16(port);
  if (::inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0) {
    LOG_SYSERR << "sockets::fromIpPort";
  }
}

int getSockError(int sockfd) {
  int optval;
  socklen_t optlen = static_cast<socklen_t>(sizeof(optval));
  if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
    return errno;
  } else {
    return optval;
  }
}

struct sockaddr_in6 getLocalAddr(int sockfd) {
  struct sockaddr_in6 localAddr;
  bzero(&localAddr, sizeof(localAddr));
  socklen_t addrlen = static_cast<socklen_t>(sizeof(localAddr));
  if (::getsockname(sockfd, sockaddr_cast(&localAddr), &addrlen) < 0) {
    LOG_SYSERR << "sockets::getLocalAddr";
  }
  return localAddr;
}

struct sockaddr_in6 getPeerAddr(int sockfd) {
  struct sockaddr_in6 peerAddr;
  bzero(&peerAddr, sizeof(peerAddr));
  socklen_t addrlen = static_cast<socklen_t>(sizeof(peerAddr));
  if (::getpeername(sockfd, sockaddr_cast(&peerAddr), &addrlen) < 0) {
    LOG_SYSERR << "sockets::getPeerAddr";
  }
  return peerAddr;
}

bool isSelfConnect(int sockfd) {
  struct sockaddr_in6 localAddr = getLocalAddr(sockfd);
  struct sockaddr_in6 peerAddr = getPeerAddr(sockfd);
  if (localAddr.sin6_family == AF_INET) {
    const struct sockaddr_in* laddr4 = reinterpret_cast<struct sockaddr_in*>(&localAddr);
    const struct sockaddr_in* paddr4 = reinterpret_cast<struct sockaddr_in*>(&peerAddr);
    return laddr4->sin_port == paddr4->sin_port
        && laddr4->sin_addr.s_addr == paddr4->sin_addr.s_addr;
  }
  if (localAddr.sin6_family == AF_INET6) {
    return localAddr.sin6_port == peerAddr.sin6_port
        && !memcmp(&localAddr.sin6_addr, &peerAddr.sin6_addr, sizeof(localAddr.sin6_addr));
  }
  return false;
}

}  // namespace sockets
}  // namespace net
}  // namespace tnet
