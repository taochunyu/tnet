#ifndef TNET_BASE_TIMERFD_H
#define TNET_BASE_TIMERFD_H

#include <tnet/base/Timestamp.h>
#include <time.h>

namespace tnet {
namespace timerfd {

int createTimerfd();
struct timespec howMuchTimeFromNow(Timestamp when);
void readTimerFd(int fd, Timestamp now);
void resetTimerFd(int timerfd, Timestamp expiration);

}  // namespace timerfd
}  // namespace tnet

#endif  // TNET_BASE_TIMERFD_H
