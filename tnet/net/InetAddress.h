#ifndef TNET_NET_INETADDRESS_H
#define TNET_NET_INETADDRESS_H

#include <tnet/base/copyable.h>
#include <tnet/base/StringPiece.h>
#include <tnet/net/SocketsOps.h>

#include <netinet/in.h>

namespace tnet {
namespace net {

class InetAddress : tnet::copyable {
 public:
  explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false, bool ipv6 = false);
  InetAddress(StringArg ip, uint16_t port, bool ipv6 = false);
  explicit InetAddress(const struct sockaddr_in& addr) : _addr(addr) {}
  explicit InetAddress(const struct sockaddr_in6& addr) : _addr6(addr) {}

  sa_family_t family() const { return _addr.sin_family; }
  std::string toIp() const;
  std::string toIpPort() const;
  uint16_t toPort() const;

  //const struct sockaddr* getSockAddr() const { return sockets::sockaddr_cast(&_addr6); }
  const struct sockaddr* getSockAddr() const {
    return (const struct sockaddr*)&te;
  }
  void setSockAddr6(const struct sockaddr_in6& addr) { _addr6 = addr; }

  uint32_t ipNetEndian() const;
  uint16_t portNetEndian() const { return _addr.sin_port; }

  static bool resolve(StringArg hostname, InetAddress* out);
 private:
  union {
    struct sockaddr_in _addr;
    struct sockaddr_in6 _addr6;
  };
  struct sockaddr_in te;
};

}  // net
}  // tnet

#endif  // TNET_NET_INETADDRESS_H
