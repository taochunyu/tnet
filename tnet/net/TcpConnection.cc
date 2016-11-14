#include <tnet/net/TcpConnection.h>

#include <tnet/base/Logging.h>
#include <tnet/net/Callbacks.h>
#include <tnet/net/Channel.h>
#include <tnet/net/Socket.h>
#include <tnet/net/SocketsOps.h>
#include <tnet/net/EventLoop.h>
#include <tnet/net/InetAddress.h>
#include <tnet/net/TimerId.h>

#include <errno.h>

using namespace tnet;
using namespace tnet::net;

void tnet::net::defaultConnectionCallback(const TcpConnectionPtr& conn) {
  LOG_INFO << "defaultConnectionCallback";
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

TcpConnection::TcpConnection(EventLoop* loop,
                             const std::string& nameArg,
                             int sockfd,
                             const InetAddress& localAddr,
                             const InetAddress& peerAddr)
    : _loop(loop),
      _name(nameArg),
      _state(kConnecting),
      _socket(new Socket(sockfd)),
      _channel(new Channel(loop, sockfd)),
      _localAddress(localAddr),
      _peerAddress(peerAddr),
      _highWaterMark(64 * 1024 * 1024),
      _reading(true) {
  _channel->onReadable([this](Timestamp receiveTime) {
    handleRead(receiveTime);
  });
  _channel->onWritable([this]{ handleWrite(); });
  _channel->onClose([this]{ handleClose(); });
  _channel->onError([this]{ handleError(); });

  LOG_DEBUG
    << "TcpConnection::ctor[" << _name << "] at " << this
    << " fd=" << sockfd;

  _socket->setKeepAlive(true);
}

TcpConnection::~TcpConnection() {
  LOG_INFO
    << "TcpConnection::dtor[" << _name << "] at " << this
    << " fd=" << _channel->fd()
    << " state=" << stateToString();
  assert(_state == kDisconnected);
}

bool TcpConnection::getTcpInfo(struct tcp_info* tcpi) const {
  return _socket->getTcpInfo(tcpi);
}

std::string TcpConnection::getTcpInfoString() const {
  char buf[1024];
  buf[0] = '\0';
  _socket->getTcpInfoString(buf, sizeof(buf));
  return buf;
}

void TcpConnection::send(const void* data, size_t len) {
  send(StringPiece(static_cast<const char*>(data), len));
}

void TcpConnection::send(const StringPiece& message) {
  if (_state == kConnected) {
    if (_loop->isInLoopThread()) {
      sendInLoop(message);
    } else {
      _loop->runInLoop([this, &message]{ sendInLoop(message); });
    }
  }
}

void TcpConnection::send(Buffer* buf) {
  if (_state == kConnected) {
    if (_loop->isInLoopThread()) {
      sendInLoop(buf->peek(), buf->readableBytes());
    } else {
      _loop->runInLoop([this, buf]{
        sendInLoop(buf->peek(), buf->readableBytes());
      });
    }
  }
}

void TcpConnection::sendInLoop(const StringPiece& message) {
  sendInLoop(message.data(), message.size());
}

void TcpConnection::sendInLoop(const void* data, size_t len) {
  _loop->assertInLoopThread();
  ssize_t nwrote = 0;
  size_t remaining = len;
  bool faultError = false;
  if (_state == kDisconnected) {
    LOG_WARN << "disconnected, give up writing";
    return;
  }
  // if nothing in output queue, try writing directly
  if (!(_channel->isWriting() && _outputBuffer.readableBytes() == 0)) {
    nwrote = sockets::write(_channel->fd(), data, len);
    if (nwrote >= 0) {
      remaining = len - nwrote;
      if (remaining == 0 && _writeCompletedCallback) {
        _loop->queueInLoop([this]{
          _writeCompletedCallback(shared_from_this());
        });
      }
    } else {
      nwrote = 0;
      if (errno != EWOULDBLOCK) {
        LOG_SYSERR << "TcpConnection::sendInLoop";
        if (errno == EPIPE || errno == ECONNRESET) {
          faultError = true;
        }
      }
    }
  }

  assert(remaining <= len);
  if (!faultError && remaining > 0) {
    size_t oldLen = _outputBuffer.readableBytes();
    if (oldLen + remaining >= _highWaterMark
          && oldLen < _highWaterMark
          && _highWaterMarkCallback) {
        _loop->queueInLoop([this, oldLen, remaining]{
          _highWaterMarkCallback(shared_from_this(), oldLen + remaining);
        });
    }
    _outputBuffer.append(static_cast<const char*>(data) + nwrote, remaining);
    if (!_channel->isWriting()) {
      _channel->enableWriting();
    }
  }
}

void TcpConnection::shutdown() {
  if (_state == kConnected) {
    setState(kDisconnecting);
    _loop->runInLoop([this]{ shutdownInLoop(); });
  }
}

void TcpConnection::shutdownInLoop() {
  _loop->assertInLoopThread();
  if (!_channel->isWriting()) {
    _socket->shutdownWrite();
  }
}

void TcpConnection::forceClose() {
  if (_state == kConnected || _state == kDisconnecting) {
    setState(kDisconnecting);
    _loop->queueInLoop([this] { forceCloseInLoop(); });
  }
}

void TcpConnection::forceCloseWithDelay(double seconds) {
  if (_state == kConnected || _state == kDisconnecting) {
    setState(kDisconnecting);
    _loop->runAfter(seconds, [this]{ forceClose(); });
  }
}

void TcpConnection::forceCloseInLoop() {
  _loop->assertInLoopThread();
  if (_state == kConnected || _state == kDisconnecting) {
    handleClose();
  }
}

const char* TcpConnection::stateToString() const {
  switch (_state) {
    case kDisconnected:
      return "kDisconnected";
    case kConnecting:
      return "kConnecting";
    case kConnected:
      return "kConnected";
    case kDisconnecting:
      return "kDisconnecting";
  }
  return "unknown state";
}

void TcpConnection::setTcpNoDelay(bool on) {
  _socket->setTcpNoDelay(on);
}

void TcpConnection::startRead() {
  _loop->runInLoop([this]{ startReadInLoop(); });
}

void TcpConnection::startReadInLoop() {
  _loop->assertInLoopThread();
  if (!_reading || !_channel->isReading()) {
    _channel->enableReading();
    _reading = true;
  }
}

void TcpConnection::stopRead() {
  _loop->runInLoop([this]{ stopReadInLoop(); });
}

void TcpConnection::stopReadInLoop() {
  _loop->assertInLoopThread();
  if (_reading || _channel->isReading()) {
    _channel->disableReading();
    _reading = false;
  }
}

void TcpConnection::establishConnection() {
  _loop->assertInLoopThread();
  LOG_INFO << "TcpConnection::establishConnection";
  assert(_state == kConnecting);
  setState(kConnected);
  _channel->tie(shared_from_this());
  _channel->enableReading();
  _connectionCallback(shared_from_this());
}

void TcpConnection::destoryConnection() {
  _loop->assertInLoopThread();
  if (_state == kConnected) {
    setState(kDisconnected);
    _channel->disableAll();
  }
  _channel->remove();
}

void TcpConnection::handleRead(Timestamp receiveTime) {
  _loop->assertInLoopThread();
  int saveErrno = 0;
  ssize_t n = _inputBuffer.readFdIntoBuffer(_channel->fd(), &saveErrno);
  if (n > 0) {
    _messageCallback(shared_from_this(), &_inputBuffer, receiveTime);
  } else if (n == 0) {
    handleClose();
  } else {
    errno = saveErrno;
    LOG_SYSERR << "TcpConnection::handleRead";
    handleError();
  }
}

void TcpConnection::handleWrite() {
  _loop->assertInLoopThread();
  if (_channel->isWriting()) {
    ssize_t n = sockets::write(_channel->fd(),
                               _outputBuffer.peek(),
                               _outputBuffer.readableBytes());
    if (n > 0) {
      _outputBuffer.retrieve(n);
      if (_outputBuffer.readableBytes() == 0) {
        _channel->disableWriting();
        if (_writeCompletedCallback) {
          _loop->queueInLoop([this]{
            _writeCompletedCallback(shared_from_this());
          });
        }
        if (_state == kDisconnecting) {
          shutdownInLoop();
        }
      }
    } else {
      LOG_SYSERR << "TcpConnection::handleWrite";
    }
  } else {
    LOG_TRACE
      << "Connection fd = " << _channel->fd()
      << " is down, no more writing";
  }
}

void TcpConnection::handleClose() {
  _loop->assertInLoopThread();
  LOG_TRACE << "fd = " << _channel->fd() << " state = " << stateToString();
  assert(_state == kConnected || _state == kDisconnecting);
  setState(kDisconnected);
  _channel->disableAll();
  _closeCallback(shared_from_this());
}

void TcpConnection::handleError() {
  int err = sockets::getSockError(_channel->fd());
  LOG_ERROR
    << "TcpConnection::handleError [" << _name
    << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}
