#include <tnet/base/Logging.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <cassert>
#include <sstream>

namespace tnet {

thread_local char t_errnobuf[512];
thread_local char t_time[32];
thread_local time_t t_lastSecond;

Logger::LogLevel initLogLevel() {
  if (::getenv("TNET_LOG_TRACE")) {
    return Logger::TRACE;
  } else if (::getenv("TNET_LOG_DEBUG")) {
    return Logger::DEBUG;
  } else {
    return Logger::INFO;
  }
}

Logger::LogLevel g_loglevel = initLogLevel();

const char* LogLevelName[Logger::NUM_LOG_LEVEL] = {
  "TRACE ",
  "DEBUG ",
  "INFO  ",
  "WARN  ",
  "ERROR ",
  "FATAL "
};

class T {
 public:
  T(const char *str, std::size_t len) : _str(str), _len(len) {
    assert(strlen(_str) == _len);
  }
  const char       *_str;
  const std::size_t _len;
};

inline LogStream& operator<<(LogStream &ls, T v) {
  ls.append(v._str, v._len);
  return ls;
}

inline LogStream& operator<<(LogStream &ls, Logger::SourceFile &file) {
  ls.append(file.data(), file.size());
  return ls;
}

void defaultOutput(const char* msg, int len) {
  std::size_t n = fwrite(msg, 1, len, stdout);
  (void)n;
}

void defaultFlush() {
  fflush(stdout);
}

std::function<void(const char*, int)> g_output = defaultOutput;
std::function<void()> g_flush = defaultFlush;

Logger::Impl::Impl(LogLevel level, int savedErrno, const SourceFile &file, int line)
  : _time(Timestamp::now()), _stream(), _level(level), _line(line), _basename(file)
{
  formatTime();

}

void Logger::Impl::formatTime() {
  int64_t microSecondsSinceEpoch = _time.microSecondsSinceEpoch();
  time_t seconds
    = static_cast<time_t>(microSecondsSinceEpoch / Timestamp::kMicroSecondsSinceEpoch);
  int microSeconds
    = static_cast<int>(microSecondsSinceEpoch % Timestamp::kMicroSecondsSinceEpoch);
  if (seconds != t_lastSecond) {
    t_lastSecond = seconds;
    struct tm tm_time;
    ::gmtime_r(&seconds, &tm_time);
    int len = snprintf(
      t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d",
      tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
      tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec
    );
    assert(len == 17);
    (void)len;
    Fmt us(".%06dZ", microSeconds);
    assert(us.length() == 9);
    _stream << T(t_time, 17) << T(us.data(), 9);
  }
}

void Logger::Impl::finish() {
  _stream << " - " << _basename << ':' << _line << '\n';
}

Logger::Logger(SourceFile file, int line) : _impl(INFO, 0, file, line) {}

Logger::Logger(SourceFile file, int line, LogLevel level) : _impl(level, 0, file, line) {}

Logger::Logger(SourceFile file, int line, LogLevel level, const char *func)
  : _impl(level, 0, file, line)
{
  _impl._stream << func << ' ';
}

Logger::Logger(SourceFile file, int line, LogLevel level, bool toAbort)
  : _impl(toAbort ? FATAL : ERROR, errno, file, line) {}

Logger::~Logger() {
  _impl.finish();
  const LogStream::Buffer& buf(stream().buffer());
  g_output(buf.data(), buf.length());
  if (_impl._level == FATAL) {
    g_flush();
    abort();
  }
}

void Logger::setLogLevel(Logger::LogLevel level) {
  g_loglevel = level;
}

void Logger::setOutput(Logger::OutputFunc out) {
  g_output = out;
}

void Logger::setFlush(FlushFunc flush) {
  g_flush = flush;
}

}  // namespace tnet
