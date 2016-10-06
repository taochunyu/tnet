#ifndef TNET_NET_ENDIAN_H
#define TNET_NET_ENDIAN_H

#include <stdint.h>
#include <machine/endian.h>

namespace tnet {
namespace net {
namespace sockets {

inline uint64_t hostToNetwork64(uint64_t host64) {
  return htonll(host64);
}

inline uint32_t hostToNetwork32(uint32_t host32) {
  return htonl(host32);
}

inline uint16_t hostToNetwork16(uint16_t host16) {
  return htons(host16);
}

inline uint64_t network64Tohost(uint64_t net64) {
  return ntohll(net64);
}

inline uint32_t network32Tohost(uint32_t net32) {
  return ntohl(net32);
}

inline uint16_t network16Tohost(uint16_t net16) {
  return ntohs(net16);
}

}  // namespace sockets
}  // namespace net
}  // namespace tnet

#endif  // TNET_NET_ENDIAN_H
