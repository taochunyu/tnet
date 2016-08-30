#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <tnet/base/copyable.h>
#include <tnet/base/Types.h>
#include <numeric>

namespace tnet {

class Timestamp : tnet::copyable {
 public:
  Timestamp() : _microSecondsSinceEpoch(0) {}
  explicit Timestamp(int64_t ms) : _microSecondsSinceEpoch(ms) {}
  void swap(Timestamp &that) {
    std::swap(_microSecondsSinceEpoch, that._microSecondsSinceEpoch);
  }
  std::string toString() const;
  std::string toFormattedString(bool showMicroSeconds = true) const;
  bool valid() const { return _microSecondsSinceEpoch > 0; }
  int64_t microSecondsSinceEpoch() const { return _microSecondsSinceEpoch; }
  time_t secondsSinceEpoch() const {
    return _microSecondsSinceEpoch / kMicroSecondsPerSecond;
  }
  static Timestamp now();
  static Timestamp invalid() { return Timestamp(); }
  static Timestamp fromUnixTime(time_t t) {
    return fromUnixTime(t, 0);
  }
  static Timestamp fromUnixTime(time_t t, int microSeconds) {
    return Timestamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond + microSeconds);
  }
  static const int kMicroSecondsPerSecond = 1000 * 1000;
 private:
  int64_t _microSecondsSinceEpoch;
};

inline bool operator<(Timestamp &lhs, Timestamp &rhs) {
  return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp &lhs, Timestamp &rhs) {
  return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

inline bool operator!=(Timestamp &lhs, Timestamp &rhs) {
  return lhs.microSecondsSinceEpoch() != rhs.microSecondsSinceEpoch();
}

inline bool operator>(Timestamp &lhs, Timestamp &rhs) {
  return lhs.microSecondsSinceEpoch() > rhs.microSecondsSinceEpoch();
}

inline bool operator<=(Timestamp &lhs, Timestamp &rhs) {
  return lhs.microSecondsSinceEpoch() <= rhs.microSecondsSinceEpoch();
}

inline bool operator>=(Timestamp &lhs, Timestamp &rhs) {
  return lhs.microSecondsSinceEpoch() >= rhs.microSecondsSinceEpoch();
}

inline double timeDifference(const Timestamp &high, const Timestamp &low) {
  int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
  return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

inline Timestamp addTime(Timestamp timestamp, double seconds) {
  int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
  return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}

}  // namespace tnet

#endif  // TIMESTAMP_H
