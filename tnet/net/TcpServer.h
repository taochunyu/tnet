#ifndef TNET_NET_TCPSERVER_H
#define TNET_NET_TCPSERVER_H

#include <tnet/base/nocopyable.h>
#include <tnet/net/Callbacks.h>
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

  /// Set the number of threads for handling input.
  /// Always accepts new connection in loop's thread.
  /// Must be called before @c start
  /// @param numThreads
  /// - 0 means all I/O in loop's thread, no thread will created.
  ///   this is the default value.
  /// - 1 means all I/O in another thread.
  /// - N means a thread pool with N threads, new connections
  ///   are assigned on a round-robin basis.

  void setThreadNum(int numThreads);
  void setThreadInitCallback(const ThreadInitCallback& cb) {
    _threadInitCallback = cb;
  }
  /// valid after calling start()
  std::shared_ptr<EventLoopThreadPool> threadPool() const {
    return _threadPool;
  }
  /// Starts the server if it's not listenning.
  /// It's harmless to call it multiple times.
  /// Thread safe.
  void start();

  /// Not thread safe.
  void onConnection(const ConnectionCallback& cb) {
    _connectionCallback = cb;
  }

  void onConnection(ConnectionCallback&& cb) {
    _connectionCallback = std::move(cb);
  }

  /// Not thread safe.
  void onMessage(const MessageCallback& cb) {
    _messageCallback = cb;
  }
  void onMessage(MessageCallback&& cb) {
    _messageCallback = std::move(cb);
  }

  /// Not thread safe.
  void onWriteCompleted(const WriteCompletedCallback& cb) {
    _writeCompletedCallback = cb;
  }
  void onWriteCompleted(WriteCompletedCallback&& cb) {
    _writeCompletedCallback = std::move(cb);
  }
 private:
  using ConnectionMap = std::map<std::string, TcpConnectionPtr>;
  /// Not thread safe, but in loop
  void newConnection(int sockfd, const InetAddress& peerAddr);
  /// Thread safe
  void removeConnection(const TcpConnectionPtr& conn);
  /// Not thread safe, but in loop
  void removeConnectionInLoop(const TcpConnectionPtr& conn);

  EventLoop* _loop;  // Acceptor loop
  const std::string _ipPort;
  const std::string _name;
  std::unique_ptr<Acceptor> _acceptor;  // avoid revealing Acceptor
  std::shared_ptr<EventLoopThreadPool> _threadPool;
  ConnectionCallback _connectionCallback;
  MessageCallback _messageCallback;
  WriteCompletedCallback _writeCompletedCallback;
  ThreadInitCallback _threadInitCallback;
  std::atomic<int> _started;

  // always in loop thread
  int _nextConnId;
  ConnectionMap _connections;
};

}  // namespace net
}  // namespace tnet

#endif  // TNET_NET_TCPSERVER_H
