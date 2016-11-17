#ifndef TNET_NET_DISPATHER_H
#define TNET_NET_DISPATHER_H

#include <tnet/base/Logging.h>
#include <tnet/base/nocopyable.h>
#include <tnet/base/Mutex.h>
#include <tnet/net/Callbacks.h>
#include <functional>
#include <atomic>
#include <map>

namespace tnet {
namespace net {

class Dispather : tnet::nocopyable {
 public:
  using Handler = std::function<void(const TcpConnectionPtr&, std::string, Timestamp)>;
  using Watcher = std::function<void(const TcpConnectionPtr&, std::string, std::string, Timestamp)>;
  Dispather() : _lck(false), _name("noname") {}
  explicit Dispather(std::string nameArg) : _lck(false), _name(nameArg) {}
  ~Dispather() {
    LOG_INFO << "Dispather " << _name << " dtor";
  }
  void use(std::string method, Handler handler) {
    if (_lck.load()) {
      LOG_ERROR << "Dispather::use watcher already started";
    }
    {
      MutexLockGuard lck(_mtx);
      if (_mapping.find(method) == _mapping.end()) {
        _mapping.emplace(method, handler);
      } else {
        LOG_ERROR << "Dispather::use method is existent";
      }
    }
  }
  Watcher watcher() {
    _lck.store(true);
    return [this] (const TcpConnectionPtr& conn, std::string method, std::string message, Timestamp receiveTime) {
      if (_mapping.find(method) != _mapping.end()) {
        _mapping[method](conn, message, receiveTime);
      }
    };
  }
 private:
  std::atomic<bool> _lck;
  std::map<std::string, Handler> _mapping;
  MutexLock _mtx;
  std::string _name;
};

}
}  // namespace tnet

#endif  // TNET_NET_DISPATHER_H
