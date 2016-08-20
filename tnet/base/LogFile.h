#ifndef TNET_BASE_LOG_FILE_H
#define TNET_BASE_LOG_FILE_H

#include <tnet/base/Types.h>
#include <tnet/base/nocopyable.h>
#include <memory>
#include <string>
#include <mutex>

namespace tnet {

namespace FileUtil {
class AppendFile;
}  // namespace tnet

class LogFile : public tnet::nocopyable {
 public:
  LogFile(
    const std::string &basename,
    std::size_t rollSize,
    bool threadSafe = true,
    int flushInterval = 3,
    int checkEveryN = 1024
  );
  ~LogFile();
  void append(const char* logLine, int len);
  void flush();
  void rollFile();
 private:
  static std::string getFileName(const std::string &basename, time_t *now);
  static const int _kRollPerSecond = 60 * 60 * 24;
  void append_unlock(const char* logLine, int len);
  const std::string _basename;
  const size_t _rollSize;
  const int _flushInterval;
  const int _checkEveryN;
  int _count;
  std::unique_ptr<std::mutex> _mtx;
  time_t _startOfPeriod;
  time_t _lastRoll;
  time_t _lastFlush;
  std::unique_ptr<FileUtil::AppendFile> _file;  
};

}  // namespace tnet

#endif  // TNET_BASE_LOG_FILE_H
