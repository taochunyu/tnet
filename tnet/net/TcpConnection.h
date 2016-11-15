#ifndef TNET_NET_TCPCONNECTION_H
#define TNET_NET_TCPCONNECTION_H

#include <tnet/base/any.h>
#include <tnet/base/nocopyable.h>
#include <tnet/base/StringPiece.h>
#include <tnet/base/Timestamp.h>
#include <tnet/net/Buffer.h>
#include <tnet/net/Callbacks.h>
#include <tnet/net/InetAddress.h>
#include <memory>
#include <string>

struct tcp_info;

namespace tnet {
namespace net {

class Channel;
class EventLoop;
class Socket;

class TcpConnection : tnet::nocopyable, public std::enable_shared_from_this<TcpConnection> {
 public:
  // User should not create this object.
  TcpConnection(EventLoop* loop,
                const std::string& name,
                int sockfd,
                const InetAddress& localAddr,
                const InetAddress& peerAddr);
  ~TcpConnection();

  EventLoop* getLoop() const { return _loop; }
  const std::string name() const { return _name; }
  const InetAddress localAddress() const { return _localAddress; }
  const InetAddress peerAddress() const { return _peerAddress; }
  bool connected() const { return _state == kConnected; }
  bool disconnected() const { return _state == kDisconnected; }
  bool getTcpInfo(struct tcp_info*) const;
  std::string getTcpInfoString() const;

  void shutdown();
  void forceClose();
  void forceCloseWithDelay(double seconds);
  void forceCloseInLoop();
  void setTcpNoDelay(bool on);

  void send(const void* message, size_t len);
  void send(const StringPiece& message);
  void send(Buffer* message);

  void startRead();
  void stopRead();
  bool isReading() const { return _reading; }

  Buffer* inputBuffer() {
    return &_inputBuffer;
  }
  Buffer* outputBuffer() {
    return &_outputBuffer;
  }

  void ctx(const tnet::any& context) { _context = context; }
  const tnet::any& ctx() const { return _context; }
  tnet::any* ctxptr() { return &_context; }

  void onConnected(const ConnectionCallback& cb) {
    _connectionCallback = cb;
  }
  void onMessage(const MessageCallback& cb) {
    _messageCallback = cb;
  }
  void onWriteCompleted(const WriteCompletedCallback& cb) {
    _writeCompletedCallback = cb;
  }
  void onHighWaterMark(const HighWaterMarkCallback& cb,
                                size_t highWaterMark) {
    _highWaterMarkCallback = cb;
    _highWaterMark = highWaterMark;
  }
  void onClose(const CloseCallback& cb) {
    _closeCallback = cb;
  }

  void establishConnection();
  void destoryConnection();
 private:
  enum StateE { kConnecting, kConnected, kDisconnecting, kDisconnected };
  void handleRead(Timestamp receiveTime);
  void handleWrite();
  void handleClose();
  void handleError();

  void shutdownInLoop();

  void sendInLoop(const StringPiece& message);
  void sendInLoop(const void* message, size_t len);

  void startReadInLoop();
  void stopReadInLoop();

  void setState(StateE s) { _state = s; }
  const char* stateToString() const;

  EventLoop* _loop;
  const std::string _name;
  StateE _state;
  std::unique_ptr<Socket> _socket;
  std::unique_ptr<Channel> _channel;
  const InetAddress _localAddress;
  const InetAddress _peerAddress;
  ConnectionCallback _connectionCallback;
  MessageCallback _messageCallback;
  WriteCompletedCallback _writeCompletedCallback;
  HighWaterMarkCallback _highWaterMarkCallback;
  CloseCallback _closeCallback;
  size_t _highWaterMark;
  Buffer _inputBuffer;
  Buffer _outputBuffer;
  tnet::any _context;
  bool _reading;
  Timestamp _createTime;
  Timestamp _lastReceiveTime;
  size_t _bytesRecevied;
  size_t _bytesSent;
};

}  // namespace net
}  // namespace tnet

#endif  // TNET_NET_TCPCONNECTION_H
