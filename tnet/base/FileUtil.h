#ifndef TNET_BASE_FILE_UTIL_H
#define TNET_BASE_FILE_UTIL_H

#include <tnet/base/StringPiece.h>
#include <tnet/base/nocopyable.h>

namespace tnet {
namespace FileUtil {

// <64kb
class ReadSmallFile : public nocopyable {
 public:
  ReadSmallFile(StringArg filename);
  ~ReadSmallFile();
  // return errno
  template<typename String>
  int readToString(
    int maxSize, String *content,
    int64_t *fileSize, int64_t *modfiyTime, int64_t *createTime
  );
  int readToBuffer(int *size);
  const char* buffer() { return _buf; }
  static const int kBufferSize = 64 * 1024;
  int err() { return _err; }
 private:
  int _fd;
  int _err;
  char _buf[kBufferSize];
};

template<typename String>
int readFile(
  StringArg filename, int maxSize, String *content,
  int64_t *fileSize = nullptr,
  int64_t *modfiyTime = nullptr,
  int64_t *createTime = nullptr)
{
  ReadSmallFile file(filename);
  return file.readToString(maxSize, content, fileSize, modfiyTime, createTime);
}

// not thread safe
class AppendFile : public nocopyable {
 public:
  explicit AppendFile(StringArg filename);
  ~AppendFile();
  void append(const char *logline, const std::size_t len);
  void flush();
  std::size_t writtenBytes() const { return _writtenBytes; }
 private:
  std::size_t write(const char *logline, const std::size_t len);
  FILE *_fp;
  char _buffer[64 * 1024];
  std::size_t _writtenBytes;
};

}  // namespace FileUtil
}  // namespace tnet

#endif  // TNET_BASE_FILE_UTIL_H
