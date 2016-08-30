#ifndef TNET_BASE_LOGGING_H
#define TNET_BASE_LOGGING_H

#include <tnet/base/LogStream.h>
#include <tnet/base/TimeStamp.h>
#include <functional>
#include <cstdio>

namespace tnet {

class Logger {
 public:
  enum LogLevel { TRACE, DEBUG, INFO, WARN, ERROR, FATAL, NUM_LOG_LEVEL };
  class SourceFile {
   public:
    template<int N>
    SourceFile(const char (&arr)[N]) : _data(arr), _size(N - 1) { // arr: ref of array
      const char* slash = strrchr(_data, '/');  // return pointer to last occurrence of '/'
      if (slash) {
        _data = slash + 1;
        _size -= static_cast<int>(_data - arr);
      }
    }
    explicit SourceFile(const char *filename) : _data(filename) {
      const char* slash = strrchr(_data, '/');
      if (slash) _data = slash + 1;
      _size = static_cast<int>(strlen(_data));
    }
    const char* data() const { return _data; }
    int size() const { return _size; }
   private:
    const char *_data;
    int        _size;
  };

  Logger(SourceFile file, int line);
  Logger(SourceFile file, int line, LogLevel level);
  Logger(SourceFile file, int line, LogLevel level, const char *func);
  Logger(SourceFile file, int line, bool toAbort);
  ~Logger();

  LogStream& stream() { return _impl._stream; }
  static LogLevel logLevel();
  static void setLogLevel(LogLevel level);
  using OutputFunc = std::function<void(const char*, int)>;
  using FlushFunc = std::function<void()>;
  static void setOutput(Logger::OutputFunc);
  static void setFlush(Logger::FlushFunc);

 private:
  class Impl {  // implement
   public:
    Impl(LogLevel level, int old_errno, const SourceFile &file, int line);
    void formatTime();
    void finish();
    Timestamp _time;
    LogStream _stream;
    LogLevel _level;
    int _line;
    SourceFile _basename;
  };
  Impl _impl;
};

extern Logger::LogLevel g_loglevel;
inline Logger::LogLevel Logger::logLevel() {  // must be inline
  return g_loglevel;
}

int strerror_tl(int savedErrno);


#define LOG_TRACE if (tnet::Logger::logLevel() <= tnet::Logger::TRACE) \
  tnet::Logger(__FILE__, __LINE__, tnet::Logger::TRACE, __func__).stream()
#define LOG_DEBUG if (tnet::Logger::logLevel() <= tnet::Logger::DEBUG) \
  tnet::Logger(__FILE__, __LINE__, tnet::Logger::DEBUG, __func__).stream()
#define LOG_INFO if (tnet::Logger::logLevel() <= tnet::Logger::INFO) \
  tnet::Logger(__FILE__, __LINE__, tnet::Logger::INFO, __func__).stream()
#define LOG_WARN tnet::Logger(__FILE__, __LINE__, tnet::Logger::WARN, __func__).stream()
#define LOG_ERROR tnet::Logger(__FILE__, __LINE__, tnet::Logger::ERROR, __func__).stream()
#define LOG_FATAL tnet::Logger(__FILE__, __LINE__, tnet::Logger::FATAL, __func__).stream()
#define LOG_SYSERR tnet::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL tnet::Logger(__FILE__, __LINE__, true).stream()

#define CHECK_NOTNULL(val) \
  ::tnet::CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be Not NULL", (val))

template<typename T>
T* CheckNotNull(Logger::SourceFile file, int line, const char *names, T *ptr) {
  if (ptr == nullptr) {
    Logger(file, line, Logger::FATAL).stream() << names;
  }
  return ptr;
}

}  // namespace tnet

#endif  // TNET_BASE_LOGGING_H
