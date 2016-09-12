#ifndef TNET_NET_SOCKETSOPS_H
#define TNET_NET_SOCKETSOPS_H

#include <arpa/inet.h>

namespace tnet {
namespace net {
namespace sockets {

// create non-blocking socket file descriptor
// abort if any error
int createNonblockingOrDie(sa_family_t);


}  // namespace sockets
}  // namespace net
}  // namespace tnet

#endif  // TNET_NET_SOCKETSOPS_H
