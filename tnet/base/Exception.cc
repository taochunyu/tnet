#include <tnet/base/Exception.h>
#include <execinfo.h>
#include <stdlib.h>

namespace tnet {

Exception::Exception(const char *msg) : _message(msg) {
  fillStackTrace();
}

Exception::Exception(const std::string &msg) : _message(msg) {
  fillStackTrace();
}

Exception::~Exception() throw() {}

const char* Exception::what() const throw() {
  return _message.c_str();
}

const char* Exception::stackTrace() const throw() {
  return _stack.c_str();
}

void Exception::fillStackTrace() {
  const int len = 200;
  void* buffer[len];
  int nptrs = ::backtrace(buffer, len);
  char** strings = ::backtrace_symbols(buffer, nptrs);
  if (strings) {
    for (int i = 0; i < nptrs; ++i) {
      _stack.append(strings[i]);
      _stack.push_back('\n');
    }
    free(strings);
  }
}

}  // namespace tnet
