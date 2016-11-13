#include <tnet/net/Connector.h>
#include <tnet/base/Logging.h>
#include <tnet/net/Channel.h>
#include <tnet/net/EventLoop.h>
#include <tnet/net/SocketsOps.h>
#include <tnet/net/TimerId.h>
#include <functional>

#include <errno.h>

using namespace tnet;
using namespace tnet::net;

Connector::Connector(EventLoop* loop, const InetAddress& serverAddr)
    : _loop(loop),
      _serverAddress(serverAddr),
      _connect(false),
      _state(kDisconnected),
      _retryDelayMs(kInitRetryDelayMs) {}

Connector::~Connector() {
  assert(!_channel);
}

void Connector::start() {
  _connect = true;
  _loop->runInLoop([this]{ startInLoop(); });
}

void Connector::startInLoop() {
  _loop->assertInLoopThread();
  assert(_state == kDisconnected);
  if (_connect) {
    connect();
  } else {
    LOG_DEBUG << " do not connect";
  }
}

void Connector::stop() {
  _connect = false;
  _loop->runInLoop([this]{ stopInLoop(); });
}

void Connector::stopInLoop() {
  _loop->assertInLoopThread();
  if (_state == kConnecting) {
    setState(kDisconnected);
    int sockfd = removeAndResetChannel();
    retry(sockfd);
  }
}

void Connector::connect() {
  int sockfd = sockets::createNonblockingOrDie(_serverAddress.family());
  int ret = sockets::connect(sockfd, _serverAddress.getSockAddr());
  int saveErrno = ret;
  switch (saveErrno) {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
      connecting(sockfd);
      break;
    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
      retry(sockfd);
      break;
    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
      LOG_SYSERR
        << "connect error in Connector::startInLoop: "
        << strerror(saveErrno);
      break;
    default:
      LOG_SYSERR
        << "Unexpected error in Connector::startInLoop: "
        << strerror(saveErrno);
      sockets::close(sockfd);
      break;
  }
}

void Connector::restart() {
  _loop->assertInLoopThread();
  setState(kDisconnected);
  _retryDelayMs = kInitRetryDelayMs;
  _connect = true;
  startInLoop();
}

void Connector::connecting(int sockfd) {
  setState(kConnecting);
  assert(!_channel);
  _channel.reset(new Channel(_loop, sockfd));
  _channel->onWritable([this]{ handleWrite(); });
  _channel->onError([this]{ handleError(); });

  _channel->enableWriting();
}

int Connector::removeAndResetChannel() {
  _channel->disableAll();
  _channel->remove();
  int sockfd = _channel->fd();
  _loop->queueInLoop([this]{ resetChannel(); });
  return sockfd;
}

void Connector::resetChannel() {
  _channel.reset();
}

void Connector::handleWrite() {
  LOG_TRACE << "Connector::handleWrite " << _state;
  if (_state == kConnecting) {
    int sockfd = removeAndResetChannel();
    int err = sockets::getSockError(sockfd);
    if (err) {
      LOG_WARN
        << "Connector::handleWrite - SO_ERROR = " << err
        << " " << strerror(err);
    } else if (sockets::isSelfConnect(sockfd)) {
      LOG_WARN << "Connector::handleWrite - Self connect";
      retry(sockfd);
    } else {
      setState(kConnected);
      if (_connect) {
        _newConnectionCallback(sockfd);
      } else {
        sockets::close(sockfd);
      }
    }
  } else {
    assert(_state == kDisconnected);
  }
}

void Connector::handleError() {
  LOG_ERROR << "Connector::handleError state=" << _state;
  if (_state == kConnecting) {
    int sockfd = removeAndResetChannel();
    int err = sockets::getSockError(sockfd);
    LOG_TRACE << "SO_ERROR = " << err << " " << strerror(err);
    retry(sockfd);
  }
}

void Connector::retry(int sockfd) {
  sockets::close(sockfd);
  setState(kDisconnected);
  if (_connect) {
    LOG_INFO
      << "Connector::retry - Retry connecting to " << _serverAddress.toIpPort()
      << " in " << _retryDelayMs << " millisecond.";
    auto that = shared_from_this();
    _loop->runAfter(_retryDelayMs/1000.0, [that, this]{ startInLoop(); });
    _retryDelayMs = std::min(_retryDelayMs * 2, kMaxRetryDelayMs);
  } else {
    LOG_DEBUG << "do not connect";
  }
}
