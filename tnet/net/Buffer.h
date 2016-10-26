#ifndef TNET_NET_BUFFER_H
#define TNET_NET_BUFFER_H

#include <tnet/base/copyable.h>
#include <tnet/base/StringPiece.h>
#include <tnet/net/Endian.h>
#include <algorithm>
#include <vector>
#include <assert.h>
#include <string.h>
#include <unistd.h>

namespace tnet {
namespace net {

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode

class Buffer : public tnet::copyable {
 public:
  static const size_t kCheapPrepend = 8;
  static const size_t kInitialSize = 1024;

  explicit Buffer(size_t initialSize = kInitialSize)
    : _buffer(kCheapPrepend + initialSize),
      _readerIndex(kCheapPrepend),
      _writerIndex(kCheapPrepend) {
    assert(readableBytes() == 0);
    assert(writableBytes() == 0);
    assert(prependableBytes() == kCheapPrepend);
 }

 void swap(Buffer& rhs) {
   _buffer.swap(rhs._buffer);
   std::swap(_readerIndex, rhs._readerIndex);
   std::swap(_writerIndex, rhs._writerIndex);
 }

 size_t readableBytes() const {
   return _writerIndex - _readerIndex;
 }

 size_t writableBytes() const {
   return _buffer.size() - _writerIndex;
 }

 size_t prependableBytes() const {
   return _readerIndex;
 }

 const char* peek() const {
   return begin() + _readerIndex;
 }

 const char* findCRLF() const {
   const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
   return crlf == beginWrite() ? nullptr : crlf;
 }

 const char* findCRLF(const char* start) const {
   assert(peek() <= start);
   assert(start <= beginWrite());
   const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF + 2);
   return crlf == beginWrite() ? nullptr : crlf;
 }
 private:
  char* begin() {
    return &(*(_buffer.begin()));
  }

  const char* begin() const {
    return &(*(_buffer.begin()));
  }

  char* beginWrite() {
    return begin() + _writerIndex;
  }

  const char* beginWrite() const {
    return begin() + _writerIndex;
  }

  void makeSpace(size_t len) {
    if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
      _buffer.resize(_writerIndex + len);
    } else {
      assert(kCheapPrepend < _readerIndex);
      size_t readable = readableBytes();
      std::copy(begin() + _readerIndex,
                begin() + _writerIndex,
                begin() + kCheapPrepend);
      _readerIndex = kCheapPrepend;
      _writerIndex = _readerIndex + readable;
      assert(readable == readableBytes());
    }
  }
  std::vector<char> _buffer;
  size_t _readerIndex;
  size_t _writerIndex;

  static const char kCRLF[];
};

}  // namespace net
}  // namespace tnet

#endif  // TNET_NET_BUFFER_H
