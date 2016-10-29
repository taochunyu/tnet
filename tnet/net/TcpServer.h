#ifndef TNET_NET_TCPSERVER_H
#define TNET_NET_TCPSERVER_H

#include <tnet/base/nocopyable.h>
#include <map>
#include <string>
#include <memory>

namespace tnet {
namespace net {

class InetAddress;
class Acceptor;
class EventLoop;
class EventLoopThreadPool;
class TcpConnection;

class TcpServer : tnet::nocopyable {
 public:
  using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
  using ThreadInitCallback = std::function<void(EventLoop*)>;
  enum Option { kNoReusePort, kReusePort };

  TcpServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const std::string& nameArg,
            Option opt = kNoReusePort);
  ~TcpServer();
  const std::string ipPort() const { return _ipPort; }
  const std::string name() const { return _name; }
  EventLoop* getLoop() const { return _loop; }

   
 private:
};

}  // namespace net
}  // namespace tnet

#endif  // TNET_NET_TCPSERVER_H
