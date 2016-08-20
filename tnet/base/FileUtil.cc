#include <tnet/base/FileUtil.h>
#include <tnet/base/Logging.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cassert>
#include <cerrno>
#include <cstdio>

namespace tnet {

FileUtil::AppendFile::AppendFile(StringArg filename)
  : _fp(::fopen(filename.c_str(). "ae")), _writtenBytes(0)
{
  assert(_fp);
  ::setbuffer(_fp, _buffer, sizeof(_buffer));
}

FileUtil::AppendFile::~AppendFile() {
  ::fclose(_fp);
}

void FileUtil::AppendFile::append(const char *logline, std::size_t len) {
  std::size_t n = write(logline, len);
  std::size_t remain = len - n;
  while (remain > 0) {
    std::size_t x = write(logline + n, remain);
    if (x == 0) {
      int err = ferror(_fp);
      if (err) {
        fprintf(stderr, "AppendFile::append() failed %d\n", strerror_tl(err));
      }
      break;
    }
    n += x;
    remain = len - n;
  }
  _writtenBytes += len;
}

void FileUtil::AppendFile::flush() {
  fflush(_fp);
}

std::size_t FileUtil::AppendFile::write(const char *logline, std::size_t len) {
  return ::fwrite_unlocked(logline, 1, len, _fp);
}

}  // namespace tnet
