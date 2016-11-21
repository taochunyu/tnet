#ifndef TNET_NET_CODEC_H
#define TNET_NET_CODEC_H

#include <tnet/base/Logging.h>
#include <tnet/base/nocopyable.h>
#include <tnet/net/Buffer.h>
#include <tnet/net/Endian.h>
#include <tnet/net/TcpConnection.h>
#include <functional>
#include <inttypes.h>

/// @code
/// |--------------64--------------|-----------len--------------|   --32---|
/// |                              |--nameLen---|               |          |
/// +--------------+---------------+------------+---------------+   -------+
/// |    headr     |     headr     |           body             |          |
/// +--------------+---------------+------------+---------------+   -------+
/// |     len      |    nameLen    |  nameData  |     data      |    check |
/// |   int32_t    |    int32_t    |    char    |     char      |   int32_t|
/// +--------------+---------------+------------+---------------+   -------+
/// |              |               |            |               |          |
/// 0             31              63       nameLen + 63     len + 63   len + 31
/// @endcode

namespace tnet {
namespace net {

class Codec : tnet::nocopyable {
 public:
  using DecodedMessageCallback
    = std::function<void(const TcpConnectionPtr& conn,
                         const std::string& method,
                         const std::string& message,
                         Timestamp time)>;

  explicit Codec(const DecodedMessageCallback& cb) : _cb(cb) {}
  explicit Codec(DecodedMessageCallback&& cb) : _cb(std::move(cb)) {}

  void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime) {
    printf("啦啦啦\n");
    while (buf->readableBytes() >= kHeaderLen) {
      // len
      const int32_t len = buf->peekInt32();
      if (len > 65535 || len < 0) {
        LOG_ERROR << "Invalid length " << len;
        conn->shutdown();
        break;
      } else if (buf->readableBytes() >= len + kHeaderLen) {
        buf->retrieve(kHeaderLen);
        // nameLen
        const int32_t nameLen = buf->peekInt32();
        if (nameLen > 65535 || nameLen < 0) {
          LOG_ERROR << "Invalid length " << nameLen;
          conn->shutdown();
          break;
        }
        buf->retrieve(kHeaderLen);
        std::string method(buf->peek(), nameLen);
        std::string message(buf->peek() + nameLen, len - nameLen);
        printf("哈哈\n");
        _cb(conn, method, message, receiveTime);
        buf->retrieve(len);
      } else {
        break;
      }
    }
  }

  void send(const TcpConnectionPtr& conn, const std::string method, const std::string message){
    Buffer buf;
    buf.append(method.data(), method.size());
    buf.append(message.data(), message.size());
    int32_t nameLen = method.size();
    int32_t len = method.size() + message.size();
    buf.prependInt32(nameLen);
    buf.prependInt32(len);
    conn->send(&buf);
  }
 private:
  const static size_t kHeaderLen = sizeof(int32_t);
  DecodedMessageCallback _cb;
};

}  // namespace net
}  // namespace tnet
#endif  // TNET_NET_CODEC_H
