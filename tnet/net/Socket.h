#ifndef TNET_NET_SOCKET_H
#define TNET_NET_SOCKET_H

#include <tnet/base/nocopyable.h>

struct tcp_info;

namespace tnet {
namespace net {

class InetAddress;

class Socket : tnet::nocopyable {
 public:
  explicit Socket(int sockfd) : _sockfd(sockfd) {}
  ~Socket();

  int fd() const { return _sockfd; }
  // return true if success
  bool getTcpInfo(struct tcp_info*) const;
  bool getTcpInfoString(char* buf, int len) const;

  // abort if adress in use
  void bindAddress(const InetAddress&);
  // abort if adress in use
  void listen();

  // On success, returns a non-negative integer that is a descriptor for the
  // accept socket, which has been set to non-blocking and close-on-exec.
  // *peeraddr is assigned;
  // On error, -1 is returned, and *peeraddr is untouched.
  int accept(InetAddress* peeraddr);

  void shutdownWrite();

  // Enable/Disable TCP_NODELY (Disable/Enable Nagle's algorithm)
  void setTcpNoDelay(bool on);
  // Enable/Disable SO_REUSEADDR
  void setReuseAddr(bool on);
  // Enable/Disable SO_REUSEPORT
  void setReusePort(bool on);
  // Enable/Disable SO_KEEPALIVE
  void setKeepAlive(bool on);
 private:
  const int _sockfd;
};

}  // namespace net
}  // namespace tnet

#endif  // TNET_NET_SOCKET_H
