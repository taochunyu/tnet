#include <tnet/base/Timestamp.h>
#include <sys/time.h>
#include <cstdio>
#include <string>

#include <cinttypes>
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

namespace tnet {

std::string Timestamp::toString() const {
  char buf[32] = {0};
  int64_t seconds = _microSecondsSinceEpoch / kMicroSecondsPerSecond;
  int64_t microSeconds = _microSecondsSinceEpoch % kMicroSecondsPerSecond;
  snprintf(buf, sizeof(buf) - 1, "%" PRId64 "%.6" PRId64 "", seconds, microSeconds);
  return std::string(buf);
}

std::string Timestamp::toFormattedString(bool showMicroSeconds) const {
  char buf[32] = {0};
  time_t seconds = microSecondsSinceEpoch() / kMicroSecondsPerSecond;
  struct tm tm_time;
  gmtime_r(&seconds, &tm_time);
  if (showMicroSeconds) {
    int microSeconds = microSecondsSinceEpoch() % kMicroSecondsPerSecond;
    snprintf(
      buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%6d",
      tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
      tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
      microSeconds
    );
  } else {
    snprintf(
      buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
      tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
      tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec
    );
  }
  return std::string(buf);
}

Timestamp Timestamp::now() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  int64_t seconds = tv.tv_sec;
  return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}

}
