#ifndef TNET_NET_DISPATHER_H
#define TNET_NET_DISPATHER_H

#include <tnet/base/Logging.h>
#include <tnet/base/nocopyable.h>
#include <tnet/base/Mutex.h>
#include <tnet/net/Callbacks.h>
#include <tnet/net/EventLoop.h>
#include <tnet/net/TcpConnection.h>
#include <functional>
#include <atomic>
#include <vector>
#include <map>

namespace tnet {
namespace net {

struct Ctx {
  const TcpConnectionPtr& conn;
  EventLoop*              loop;
  std::string             method;
  std::string             message;
  Timestamp               receiveTime;
};

class Dispather : tnet::nocopyable {
 public:
  using Handler = std::function<void(Ctx)>;
  using Watcher = std::function<void(const TcpConnectionPtr&, std::string, std::string, Timestamp)>;
  Dispather() : _isWatching(false), _name("noname") {}
  explicit Dispather(std::string nameArg) : _isWatching(false), _name(nameArg) {}
  ~Dispather() {
    LOG_INFO << "Dispather " << _name << " dtor";
  }
  void use(Handler handler) {
    if (_isWatching.load()) {
      LOG_ERROR << "Dispather::use watcher already started";
    }
    {
      MutexLockGuard lck(_mtx);
      _useList.push_back(handler);
    }
  }
  void route(std::string method, Handler handler) {
    if (_isWatching.load()) {
      LOG_ERROR << "Dispather::use watcher already started";
    }
    if (_routeList.find(method) == _routeList.end()){
      MutexLockGuard lck(_mtx);
      _routeList.emplace(method, handler);
    } else {
      LOG_ERROR << "Dispather::method is already existent";
    }
  }
  void configDone() {
    _isWatching.store(true);
  }
  Watcher watcher() {
    return [this] (const TcpConnectionPtr& conn, std::string method, std::string message, Timestamp receiveTime) {
      auto loop = conn->getLoop();
      Ctx ctx = { conn, loop, method, message, receiveTime };
      for (auto it = _useList.begin(); it != _useList.end(); it++) {
        (*it)(ctx);
      }
      if (_routeList.find(method) != _routeList.end()) {
        _routeList[method](ctx);
      }
    };
  }
 private:
  std::atomic<bool> _isWatching;
  std::vector<Handler> _useList;
  std::map<std::string, Handler> _routeList;
  MutexLock _mtx;
  std::string _name;
};

}
}  // namespace tnet

#endif  // TNET_NET_DISPATHER_H
