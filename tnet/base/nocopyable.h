#ifndef TNET_BASE_NOCOPYABLE_H
#define TNET_BASE_NOCOPYABLE_H

namespace tnet {

class nocopyable {
 protected:
  nocopyable() = default;
  ~nocopyable() = default;
 private:
  nocopyable(const nocopyable&) = delete;
  nocopyable& operator=(const nocopyable) = delete;
};

}

#endif  // TNET_BASE_NOCOPYABLE_H
