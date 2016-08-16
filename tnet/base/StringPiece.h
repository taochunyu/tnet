#ifndef TNET_BASE_STRING_PIECE_H
#define TNET_BASE_STRING_PIECE_H
// 提供一个｛首字符指针，长度｝的窗口，只用4字节指针开销完成读取字符串的操作
// 字符串生命周期由使用者维护
#include <tnet/base/Types.h>
#include <cstring>
#include <iosfwd>
#include <string>

namespace tnet {

class StringArg {
 public:
  StringArg(const char *str) : _str(str) {}
  StringArg(const std::string &str) : _str(str.c_str()) {}
  const char* c_str() const { return _str; }
 private:
  const char *_str;
};

class StringPiece {
 public:
  StringPiece()
    : _strp(nullptr), _length(0) {}
  StringPiece(const char *str)
    : _strp(str), _length(static_cast<int>(strlen(str))) {}
  StringPiece(const std::string &str)
    : _strp(str.data()), _length(static_cast<int>(str.size())) {}
  StringPiece(const char *offset, int len)
    : _strp(offset), _length(len) {}

  const char* data() const { return _strp; }
  int size() const { return _length; }
  bool empty() const { return _length == 0; }
  const char* begin() { return _strp; }
  const char* end() { return _strp + _length; }
  void clear() { _strp = nullptr; _length = 0; }
  void set(const char *buffer) {
    _strp = buffer;
    _length = static_cast<int>(strlen(buffer));
  }
  void set(const char *buffer, int len) {
    _strp = buffer;
    _length = len;
  }
  char operator[](int i) const { return _strp[i]; }
  bool operator==(const StringPiece &rhs) const {
    return ((_length == rhs._length) && (memcmp(_strp, rhs._strp, _length) == 0));
  }
  bool operator!=(const StringPiece &rhs) const {
    return !(*this == rhs);
  }
  int compare(const StringPiece &str) const {
    int r = memcmp(_strp, str._strp, _length < str._length ? _length : str._length);
    if (r == 0) {
      if (_length > str._length) {
        r = 1;
      } else if (_length < str._length) {
        r = -1;
      }
    }
    return r;
  }
  std::string std_string() const {
    return std::string(_strp, _length);
  }
  void copyToStdString(std::string *target) {
    target -> assign(_strp, _length);
  }
  bool starts_with(const StringPiece &str) const {
    return ((_length >= str._length) && (memcmp(_strp, str._strp, str._length) == 0));
  }
  void remove_prefix(int n) {
    _strp += n;
    _length -= n;
  }
  void remove_suffix(int n) {
    _length -= n;
  }
 private:
  const char *_strp;
  int         _length;
};

}

std::ostream& operator<<(std::ostream &os, const tnet::StringPiece &piece);

#endif  // TNET_BASE_STRING_PIECE_H
