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

  const char* findEOL() const {
    const void* eol = memchr(peek(), '\n', readableBytes());
    return static_cast<const char*>(eol);
  }

  const char* findEOL(const char* start) const {
    assert(peek() <= start);
    assert(start <= beginWrite());
    const void* eol = memchr(peek(), '\n', readableBytes());
    return static_cast<const char*>(eol);
  }

  // retrieve returns void, to prevent
  // string str(retrieve(readableBytes()), readableBytes());
  // the evaluation of two functions are unspecified
  void retrieve(size_t len) {
    assert(len <= readableBytes());
    if (len < readableBytes()) {
      _readerIndex += len;
    } else {
      retrieveAll();
    }
  }

  void retrieveUntil(const char* end) {
    assert(peek() <= end);
    assert(end <= beginWrite());
    retrieve(end - peek());
  }

  void retrieveAll() {
    _readerIndex = kCheapPrepend;
    _writerIndex = kCheapPrepend;
  }

  void retrieveInt64() {
    retrieve(sizeof(int64_t));
  }

  void retrieveInt32() {
    retrieve(sizeof(int32_t));
  }

  void retrieveInt16() {
    retrieve(sizeof(int16_t));
  }

  void retrieveInt8() {
    retrieve(sizeof(int8_t));
  }

  std::string retrieveAsString(size_t len) {
    assert(len <= readableBytes());
    std::string result(peek(), len);
    retrieve(len);
    return result;
  }

  std::string retrieveAllAsString() {
   return retrieveAsString(readableBytes());;
  }

  StringPiece toStringPiece() const {
    return StringPiece(peek(), static_cast<int>(readableBytes()));
  }

  void append(const StringPiece& str) {
    append(str.data(), str.size());
  }
  void append(const char* data, size_t len) {
    ensureWritableBytes(len);
    std::copy(data, data + len, beginWrite());
    hasWriten(len);
  }

  void append(const void* data, size_t len) {
    append(static_cast<const char*>(data), len);
  }

  // Append inttypes using network endian
  void appendInt64(int64_t x) {
    int64_t be64 = sockets::hostToNetwork64(x);
    append(&be64, sizeof(be64));
  }

  void appendInt32(int32_t x) {
    int32_t be32 = sockets::hostToNetwork32(x);
    append(&be32, sizeof(be32));
  }

  void appendInt16(int16_t x) {
    int16_t be16 = sockets::hostToNetwork16(x);
    append(&be16, sizeof(be16));
  }

  void appendInt8(int8_t x) {
    append(&x, sizeof(x));
  }

  // Read inttypes from network endian

  int64_t readInt64() {
    int64_t result = peekInt64();
    retrieveInt64();
    return result;
  }

  int32_t readInt32() {
    int32_t result = peekInt32();
    retrieveInt32();
    return result;
  }

  int16_t readInt16() {
    int16_t result = peekInt16();
    retrieveInt16();
    return result;
  }

  int8_t readInt8() {
    int8_t result = peekInt8();
    retrieveInt8();
    return result;
  }

  const char* peek() const {
    return begin() + _readerIndex;
  }

  // Peek inttypes from network endian

  int64_t peekInt64() const {
    assert(readableBytes() >= sizeof(int64_t));
    int64_t be64 = 0;
    ::memcpy(&be64, peek(), sizeof(be64));
    return sockets::networkToHost64(be64);
  }

  int64_t peekInt32() const {
    assert(readableBytes() >= sizeof(int32_t));
    int32_t be32 = 0;
    ::memcpy(&be32, peek(), sizeof(be32));
    return sockets::networkToHost32(be32);
  }

  int64_t peekInt16() const {
    assert(readableBytes() >= sizeof(int16_t));
    int16_t be16 = 0;
    ::memcpy(&be16, peek(), sizeof(be16));
    return sockets::networkToHost16(be16);
  }

  int8_t peekInt8() const {
    assert(readableBytes() >= sizeof(int8_t));
    int8_t x = *peek();
    return x;
  }

  void prependInt64(int64_t x) {
    int64_t be64 = sockets::hostToNetwork64(x);
    prepend(&be64, sizeof(be64));
  }

  void prependInt32(int32_t x) {
    int32_t be32 = sockets::hostToNetwork32(x);
    prepend(&be32, sizeof(be32));
  }

  void prependInt16(int16_t x) {
    int16_t be16 = sockets::hostToNetwork16(x);
    prepend(&be16, sizeof(be16));
  }

  void prependInt8(int8_t x) {
    prepend(&x, sizeof(x));
  }

  void prepend(const void* data, size_t len) {
    assert(len < prependableBytes());
    _readerIndex -= len;
    const char* t = static_cast<const char*>(data);
    std::copy(t, t + len, begin() + _readerIndex);
  }

  void shrink() {
    _buffer.shrink_to_fit();
  }

  size_t internalCapacity() const {
    return _buffer.capacity();
  }

  int readFdIntoBuffer(int fd, int* savedError);
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

  void ensureWritableBytes(size_t len) {
    if (len > writableBytes()) {
      makeSpace(len);
    }
    assert(len <= writableBytes());
  }

  void hasWriten(size_t len) {
    assert(len <= writableBytes());
    _writerIndex += len;
  }

  void makeSpace(size_t len) {
    if (len > (writableBytes() + (prependableBytes() - kCheapPrepend))) {
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
  static const size_t kCheapPrepend = 8;
  static const size_t kInitialSize = 1024;
};

}  // namespace net
}  // namespace tnet

#endif  // TNET_NET_BUFFER_H
