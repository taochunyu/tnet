#include <tnet/net/TcpConnection.h>

#include <tnet/base/Logging.h>
#include <tnet/net/Callbacks.h>
#include <tnet/net/Channel.h>
#include <tnet/net/Socket.h>
#include <tnet/net/SocketsOps.h>
#include <tnet/net/EventLoop.h>

#include <errno.h>

using namespace tnet;
using namespace tnet::net;

void tnet::net::defaultConnectionCallback(const TcpConnectionPtr& conn) {
  LOG_TRACE
    << conn->localAddress().toIpPort() << " -> "
    << conn->peerAddress().toIpPort() << " is "
    << (conn->connected() ? "UP" : "DOWN");
}

void tnet::net::defaultMessageCallback(const TcpConnectionPtr& conn,
                                       Buffer* buf,
                                       Timestamp receiveTime) {
  buf->retrieveAll();
}
