#ifndef TNET_NET_ACCEPTOR_H
#define TNET_NET_ACCEPTOR_H

#include <tnet/net/Channel.h>
#include <tnet/net/Socket.h>
#include <tnet/base/nocopyable.h>
#include <functional>

namespace tnet {
namespace net {

class EventLoop;
class InetAddress;

class Acceptor : tnet::nocopyable {
 public:
  using NewConnectionCallback =
    std::function<void(int sockfd, const InetAddress&)>;

  Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reusePort);
  ~Acceptor();
  void setNewConnectionCallback(const NewConnectionCallback& cb) {
    _newConnectionCallback = cb;
  }
  bool listenning() const { return _listenning; }
  void listen();
 private:
  void handleRead();

  EventLoop* _loop;
  Socket _acceptSocket;
  Channel _acceptChannel;
  NewConnectionCallback _newConnectionCallback;
  bool _listenning;
  int _idleFd;
};

}  // namespace net
}  // namespace tnet

#endif  // TNET_NET_ACCEPTOR_H
