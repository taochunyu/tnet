#ifndef TNET_NET_CONNECTOR_H
#define TNET_NET_CONNECTOR_H

#include <tnet/net/InetAddress.h>
#include <tnet/base/nocopyable.h>
#include <functional>

namespace tnet {
namespace net {

class Channel;
class EventLoop;

class Connector : tnet::nocopyable, public std::enable_shared_from_this<Connector> {
 public:
  using NewConnectionCallback = std::function<void(int sockfd)>;
  Connector(EventLoop* loop, const InetAddress& serverAddr);
  ~Connector();

  void onConnection(const NewConnectionCallback& cb) {
    _newConnectionCallback = cb;
  }

  void start();
  void restart();
  void stop();

  const InetAddress serverAddress() const {
    return _serverAddress;
  }
 private:
  enum State { kDisconnected, kConnecting, kConnected };

  static const int kMaxRetryDelayMs = 30 * 1000;
  static const int kInitRetryDelayMs = 500;

  void setState(State s) { _state = s; }
  void startInLoop();
  void stopInLoop();
  void connect();
  void connecting(int sockfd);
  void handleWrite();
  void handleError();
  void retry(int sockfd);
  int removeAndResetChannel();
  void resetChannel();

  EventLoop* _loop;
  InetAddress _serverAddress;
  std::atomic<bool> _connect;
  std::atomic<State> _state;
  std::unique_ptr<Channel> _channel;
  NewConnectionCallback _newConnectionCallback;
  int _retryDelayMs;
};

}
}


#endif  // TNET_NET_CONNECTOR_H
