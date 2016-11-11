#include <tnet/net/Buffer.h>
#include <tnet/net/SocketsOps.h>

#include <errno.h>
#include <sys/uio.h>

using namespace tnet;
using namespace tnet::net;

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

int Buffer::readFdIntoBuffer(int fd, int* savedError) {
  char extrabuf[65536];
  struct iovec vec[2];
  const size_t writable = writableBytes();
  vec[0].iov_base = begin() + _writerIndex;
  vec[0].iov_len = writable;
  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof(extrabuf);

  const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
  const ssize_t n = sockets::readv(fd, vec, iovcnt);
  if (n < 0) {
    *savedError = errno;
  } else if (implicit_cast<size_t>(n) <= writable) {
    _writerIndex += n;
  } else {
    _writerIndex = _buffer.size();
    append(extrabuf, n - writable);
  }
  return n;
}
