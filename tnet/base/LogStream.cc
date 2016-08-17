#include "LogStream.h"
#include <cassert>

namespace tnet {
namespace detail {

const char digits[] = "9876543210123456789";
const char *zero = digits + 9;
const char disgitsHex[] = "0123456789ABCDEF";

// Efficient Integer to String Conversions, by Matthew Wilson.
template<typename T>
std::size_t convert(char buf[], T value) {
  T i = value;
  char *p = buf;
  do {
    int lsd = static_cast<int>(i % 10);
    i /= 10;
    *(p++) = zero[lsd];
  } while (i != 0);
  if (value < 0) {
    *(p++) = '-';
  }
  *p = '\0';
  std::reverse(buf, p);
  return p - buf;
}
std::size_t convertHex(char buf[], uintptr_t value) {
  uintptr_t i = value;  // 指针地址类型
  char *p = buf;
  do {
    int lsd = static_cast<int>(i % 16);
    i /= 16;
    *(p++) = disgitsHex[lsd];
  } while (i != 0);
  *p = '\0';
  std::reverse(buf, p);
  return p - buf;
}

const int kLargeBuffer = 4000 * 1000;

template class FixedBuffer<kSmallBuffer>;
template class FixedBuffer<kLargeBuffer>;

}  // detail
}  // tnet

template<typename T>
void tnet::LogStream::formatIntger(T v) {
  if (_buffer.avail() >= kMaxNumericSize) {
    std::size_t len = detail::convert(_buffer.current(), v);
    _buffer.add(len);
  }
}

namespace tnet {

tnet::LogStream& tnet::LogStream::operator<<(bool v) {
  if (v) {
    _buffer.append("true", 4);
  } else {
    _buffer.append("false", 5);
  }
  return *this;
}

LogStream& LogStream::operator<<(short v) {
  *this << static_cast<int>(v);
  return *this;
}
LogStream& LogStream::operator<<(unsigned short v) {
  *this << static_cast<unsigned int>(v);
  return *this;
}
LogStream& LogStream::operator<<(int v) {
  formatIntger(v);
  return *this;
}
LogStream& LogStream::operator<<(unsigned int v) {
  formatIntger(v);
  return *this;
}
LogStream& LogStream::operator<<(long v) {
  formatIntger(v);
  return *this;
}
LogStream& LogStream::operator<<(unsigned long v) {
  formatIntger(v);
  return *this;
}
LogStream& LogStream::operator<<(long long v) {
  formatIntger(v);
  return *this;
}
LogStream& LogStream::operator<<(unsigned long long v) {
  formatIntger(v);
  return *this;
}
LogStream& LogStream::operator<<(const void *p) {
  uintptr_t v = reinterpret_cast<uintptr_t>(p);
  if (_buffer.avail() >= kMaxNumericSize) {
    char *buf = _buffer.current();
    buf[0] = '0';
    buf[1] = 'x';
    std::size_t len = detail::convertHex(buf + 2, v);
    _buffer.add(len + 2);
  }
  return *this;
}
LogStream& LogStream::operator<<(float v) {
  *this << static_cast<double>(v);
  return *this;
}
LogStream& LogStream::operator<<(double v) {
  if (_buffer.avail() >= kMaxNumericSize) {
    int len = snprintf(_buffer.current(), kMaxNumericSize, "%.12g", v);
    _buffer.add(len);
  }
  return *this;
}
LogStream& LogStream::operator<<(char v) {
  _buffer.append(&v, 1);
  return *this;
}
LogStream& LogStream::operator<<(const char *str) {
  if (str) {
    _buffer.append(str, strlen(str));
  } else {
    _buffer.append("(null)", 6);
  }
  return *this;
}
LogStream& LogStream::operator<<(const unsigned char *str) {
  return LogStream::operator<<(reinterpret_cast<const char*>(str));
}
LogStream& LogStream::operator<<(const std::string &v) {
  _buffer.append(v.c_str(), v.size());
  return *this;
}
LogStream& LogStream::operator<<(const StringPiece &v) {
  _buffer.append(v.data(), v.size());
  return *this;
}
LogStream& LogStream::operator<<(const Buffer &v) {
  *this << v.toStringPiece();
  return *this;
}


template<typename T>
Fmt::Fmt(const char *fmt, T value) {
  _length = snprintf(_buf, sizeof(_buf), fmt, value);
  assert(static_cast<std::size_t>(_length) < sizeof(_buf));
}

// Explicit instantiations  no need to be copied
template Fmt::Fmt(const char* fmt, char);

template Fmt::Fmt(const char* fmt, short);
template Fmt::Fmt(const char* fmt, unsigned short);
template Fmt::Fmt(const char* fmt, int);
template Fmt::Fmt(const char* fmt, unsigned int);
template Fmt::Fmt(const char* fmt, long);
template Fmt::Fmt(const char* fmt, unsigned long);
template Fmt::Fmt(const char* fmt, long long);
template Fmt::Fmt(const char* fmt, unsigned long long);

template Fmt::Fmt(const char* fmt, float);
template Fmt::Fmt(const char* fmt, double);


}  // namespace tnet
