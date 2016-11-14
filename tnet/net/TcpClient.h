#ifndef TNET_NET_TCPCLIENT_H
#define TNET_NET_TCPCLIENT_H

#include <tnet/base/nocopyable.h>
#include <tnet/base/Mutex.h>
#include <tnet/net/TcpConnection.h>
#include <tnet/net/Callbacks.h>
#include <string>
#include <memory>
#include <atomic>
#include <map>

namespace tnet {
namespace net {

class Connector;

class TcpClient : tnet::nocopyable {
 public:
  TcpClient(EventLoop* loop,
            const InetAddress& serverAddr,
            const std::string& nameArg);
  ~TcpClient();

  void connect();
  void disconnect();
  void stop();

  TcpConnectionPtr connection() const {
    MutexLockGuard lock(_mutex);
    return _connection;
  }
  EventLoop* getLoop() const { return _loop; }
  bool retry() const { return _retry; }
  void enableRetry() { _retry = true; }
  std::string name() const { return _name; }

  void onConnection(const ConnectionCallback& cb) {
    _connectionCallback = cb;
  }
  void onMessage(const MessageCallback& cb) {
    _messageCallback = cb;
  }
  void onWriteComplete(const WriteCompletedCallback& cb) {
    _writeCompletedCallback = cb;
  }

  void onConnection(ConnectionCallback&& cb) {
    _connectionCallback = std::move(cb);
  }
  void onMessage(MessageCallback&& cb) {
    _messageCallback = std::move(cb);
  }
  void onWriteCompleted(WriteCompletedCallback&& cb) {
    _writeCompletedCallback = std::move(cb);
  }

 private:
  void newConnection(int sockfd);
  void removeConnection(const TcpConnectionPtr& conn);

  EventLoop* _loop;
  std::shared_ptr<Connector> _connector;
  const std::string _name;
  ConnectionCallback _connectionCallback;
  MessageCallback _messageCallback;
  WriteCompletedCallback _writeCompletedCallback;
  bool _retry;
  bool _connect;
  int _nextConnId;
  mutable MutexLock _mutex;
  TcpConnectionPtr _connection;
};

}
}

#endif  // TNET_NET_TCPCLIENT_H
