#ifndef TNET_BASE_LOG_STREAM_H
#define TNET_BASE_LOG_STREAM_H

#include <tnet/base/StringPiece.h>
#include <tnet/base/nocopyable.h>
#include <tnet/base/Types.h>
#include <cstring>
#include <string>
#include <functional>

namespace tnet {
namespace detail {  // 仅内部使用

const int kSmallBuffer = 4000;

template<int SIZE>
class FixedBuffer : tnet::nocopyable {  // 定长的缓存区，有个 “游标”: _cur
 public:
  FixedBuffer() : _cur(_data) {
    setCookie(_cookieStart);
  }
  ~FixedBuffer() {
    setCookie(_cookieEnd);
  }
  void append(const char *buf, std::size_t len) {
    if (implicit_cast<std::size_t>(avail()) > len) {
      memcpy(_cur, buf, len);
      _cur += len;
    }
  }
  const char* data() const { return _data; }
  int length() const { return static_cast<int>(_cur - _data); }
  char* current() const { return _cur; }
  int avail() const { return static_cast<int>(_end() - _cur); }
  void add(std::size_t len) { _cur += len; }
  void reset() { _cur = _data; }
  void bzero() { ::bzero(_data, sizeof(_data)); }

  // for GDB
  const char* debugString() {
    *_cur = '\0';
    return _data;
  };
  void setCookie(std::function<void()> f) { _cookie = f; }

  //for unit test
  std::string toString() const { return std::string(_data, length()); }
  tnet::StringPiece toStringPiece() const { return tnet::StringPiece(_data, length()); }
 private:
  const char* _end() const { return _data + sizeof(_data); }
  static void _cookieStart() {};
  static void _cookieEnd() {};

  std::function<void()> _cookie;
  char                  _data[SIZE];
  char                  *_cur;
};

}  // namespace detail

class LogStream : tnet::nocopyable {
 public:
  using Buffer = detail::FixedBuffer<detail::kSmallBuffer>;
  void append(const char *data, int len) { _buffer.append(data, len); }
  const Buffer& buffer() const { return _buffer; }
  void resetBuffer() { _buffer.reset(); }
  LogStream& operator<<(bool);
  LogStream& operator<<(short);
  LogStream& operator<<(unsigned short);
  LogStream& operator<<(int);
  LogStream& operator<<(unsigned int);
  LogStream& operator<<(long);
  LogStream& operator<<(unsigned long);
  LogStream& operator<<(long long);
  LogStream& operator<<(unsigned long long);
  LogStream& operator<<(const void*);
  LogStream& operator<<(float);
  LogStream& operator<<(double);
  LogStream& operator<<(char);
  LogStream& operator<<(const char*);
  LogStream& operator<<(const unsigned char*);
  LogStream& operator<<(const std::string&);
  LogStream& operator<<(const StringPiece&);
  LogStream& operator<<(const Buffer&);
 private:
  // void staticCheck();  // it will
  template<typename T>
  void formatIntger(T);
  Buffer _buffer;
  static const int kMaxNumericSize = 32;
};

class Fmt {
 public:
  template<typename T>
  Fmt(const char* fmt, T val);
  const char* data() const { return _buf; }
  int length() const { return _length; }
 private:
  char _buf[32];
  int  _length;
};

inline LogStream& operator<<(LogStream& ls, const Fmt& fmt) {
  ls.append(fmt.data(), fmt.length());
  return ls;
}

}  // namespace tnet

#endif // TNET_BASE_LOG_STREAM_H
