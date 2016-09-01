#ifndef TNET_NET_CALLBACKS_H
#define TNET_NET_CALLBACKS_H

#include <tnet/base/Timestamp.h>
#include <functional>
using std::function;
#include <memory>
using std::shared_ptr;

namespace net {

class Buffer;
class TcpConnection;

using TcpConnectionPtr      = shared_ptr<TcpConnection>;
using TimerCallback         = function<void()>;
using ConnectionCallback    = function<void(const TcpConnectionPtr&)>;
using CloseCallback         = function<void(const TcpConnectionPtr&)>;
using WriteCompleteCallback = function<void(const TcpConnectionPtr&)>;
using HighWaterMarkCallback = function<void(const TcpConnectionPtr&)>;
using MessageCallback       = function<void(const TcpConnectionPtr&, Buffer *Timestamp)>;

void defaultConnectionCallback(const TcpConnectionPtr &conn);
void defaultMessageCallback(const TcpConnectionPtr &conn,
                            Buffer *buffer,
                            tnet::Timestamp receiveTime);

}  // namespace net

#endif  // TNET_NET_CALLBACKS_H
