#include <tnet/net/InetAddress.h>
#include <tnet/net/Endian.h>
#include <tnet/net/SocketsOps.h>
#include <tnet/base/Logging.h>

#include <netdb.h>
#include <assert.h>

using namespace tnet;
using namespace net;

//     /* Structure describing an Internet socket address.  */
//     struct sockaddr_in {
//         sa_family_t    sin_family; /* address family: AF_INET */
//         uint16_t       sin_port;   /* port in network byte order */
//         struct in_addr sin_addr;   /* internet address */
//     };

//     /* Internet address. */
//     typedef uint32_t in_addr_t;
//     struct in_addr {
//         in_addr_t       s_addr;     /* address in network byte order */
//     };

//     struct sockaddr_in6 {
//         sa_family_t     sin6_family;   /* address family: AF_INET6 */
//         uint16_t        sin6_port;     /* port in network byte order */
//         uint32_t        sin6_flowinfo; /* IPv6 flow information */
//         struct in6_addr sin6_addr;     /* IPv6 address */
//         uint32_t        sin6_scope_id; /* IPv6 scope-id */
//     };

InetAddress::InetAddress(uint16_t port, bool loopbackOnly, bool ipv6) {
  if (ipv6) {
    bzero(&_addr6, sizeof(_addr6));
    _addr6.sin6_family = AF_INET6;
    in6_addr ip = loopbackOnly ? in6addr_loopback : in6addr_any;
    _addr6.sin6_addr = ip;
    _addr6.sin6_port = sockets::hostToNetwork16(port);
  } else {
    bzero(&_addr, sizeof(_addr));
    _addr.sin_family = AF_INET;
    in_addr_t ip = loopbackOnly ? INADDR_LOOPBACK : INADDR_ANY;
    _addr.sin_addr.s_addr = sockets::hostToNetwork32(ip);
    _addr.sin_port = sockets::hostToNetwork16(port);
  }
}

InetAddress::InetAddress(StringArg ip, uint16_t port, bool ipv6) {
  if (ipv6) {
    bzero(&_addr6, sizeof(_addr6));
    sockets::fromIpPort(ip.c_str(), port, &_addr6);
  } else {
    bzero(&_addr, sizeof(_addr));
    sockets::fromIpPort(ip.c_str(), port, &_addr);
  }
}

std::string InetAddress::toIp() const {
  char buf[64] = "";
  sockets::toIp(buf, sizeof(buf), getSockAddr());
  return buf;
}

std::string InetAddress::toIpPort() const {
  char buf[64] = "";
  sockets::toIpPort(buf, sizeof(buf), getSockAddr());
  return buf;
}

uint16_t InetAddress::toPort() const {
  return sockets::networkTohost16(portNetEndian());
}

uint32_t InetAddress::ipNetEndian() const {
  assert(_addr.sin_family == AF_INET);
  return _addr.sin_addr.s_addr;
}

bool InetAddress::resolve(StringArg hostname, InetAddress* out) {
  // Though gethostbyname functions are thread-safe, still it is recommended
  // to use the getaddrinfo(3) family of functions, instead.
  assert(out != nullptr);
  struct hostent* he = gethostbyname(hostname.c_str());
  if (he != nullptr) {
    assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
    out->_addr.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
    return true;
  }
  LOG_SYSERR << "InetAddress::resolve";
  return false;
}
