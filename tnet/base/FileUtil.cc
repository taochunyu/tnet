#include <tnet/base/FileUtil.h>
// #include <tnet/base/Logging.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cassert>
#include <cerrno>
#include <cstdio>

namespace tnet {

FileUtil::AppendFile::AppendFile(StringArg filename)
  : _fp(::fopen(filename.c_str(), "ae")), _writtenBytes(0)
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
        // fprintf(stderr, "AppendFile::append() failed %d\n", strerror_tl(err));
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
  return ::fwrite(logline, 1, len, _fp);
}

FileUtil::ReadSmallFile::ReadSmallFile(StringArg filename)
  : _fd(::open(filename.c_str(), O_RDONLY | O_CLOEXEC)), _err(0)
{
  _buf[0] = '\0';
  if (_fd < 0) {
    _err = errno;
  }
}

FileUtil::ReadSmallFile::~ReadSmallFile() {
  if (_fd > 0) {
    close(_fd);
  }
}

// retun errno
template<typename String>
int FileUtil::ReadSmallFile::readToString(
      int maxSize, String *content,
      int64_t *fileSize, int64_t *modfiyTime, int64_t *createTime
    )
{
  assert(content != nullptr);
  int err = _err;
  if (_fd >= 0) {
    content -> clear();
    if (fileSize) {
      struct stat statBuf;
      if (::fstat(_fd, &statBuf) == 0) {
        if (S_ISREG(statBuf.st_mode)) {
          *fileSize = statBuf.st_size;
        } else if (S_ISDIR(statBuf.st_mode)) {
          err = EISDIR;
        }
        if (modfiyTime) {
          *modfiyTime = statBuf.st_mtime;
        }
        if (createTime) {
          *createTime = statBuf.st_ctime;
        }
      } else {
        err = errno;
      }
    }
    while (content -> size() < implicit_cast<std::size_t>(maxSize)) {
      std::size_t toRead = std::min(
        implicit_cast<std::size_t>(maxSize) - content -> size(),
        sizeof(_buf)
      );
      ssize_t n = ::read(_fd, _buf, toRead);
      if (n > 0) {
        content -> append(_buf, n);
      } else {
        if (n < 0) {
          err = errno;
        }
        break;
      }
    }
  }
  return err;
}

int FileUtil::ReadSmallFile::readToBuffer(int *size) {
  int err = _err;
  if (_fd >= 0) {
    ssize_t n = ::pread(_fd, _buf, sizeof(_buf) - 1, 0);
    if (n >= 0) {
      if (size) {
        *size = static_cast<int>(n);
      }
      _buf[n] = '\0';
    } else {
      err = errno;
    }
  }
  return err;
}

template int FileUtil::readFile (
  StringArg, int, std::string*, int64_t*, int64_t*, int64_t*);

template int FileUtil::ReadSmallFile::readToString (
  int, std::string*, int64_t*, int64_t*, int64_t*);

}  // namespace tnet
