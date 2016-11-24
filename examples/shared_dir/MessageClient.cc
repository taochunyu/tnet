#include "MessageClient.h"

MessageClient::MessageClient(EventLoop* loop, InetAddress listenAddr, FileModelClient& fmc)
    : _loop(loop),
      _client(_loop, listenAddr, "MessageClient"),
      _dispather("MessageDispather"),
      _codec(_dispather.watcher()),
      _fmc(fmc)
{
  _client.onConnection([this](auto conn){ handleConn(conn); });
  _client.onMessage([this](auto conn, auto buf, auto receiveTime) {
    _codec.onMessage(conn, buf, receiveTime);
  });
  _dispather.route("/logup", [this](auto ctx){ logup(ctx); });
  _dispather.route("/login", [this](auto ctx){ login(ctx); });
  _dispather.configDone();
}

void MessageClient::send(std::string method, std::string message) {
  MutexLockGuard lck(_mtx);
  if (_conn) {
    _codec.send(_conn, method, message);
  } else {
    LOG_ERROR << "broken socket";
  }
}

void MessageClient::handleConn(const TcpConnectionPtr& conn) {
  LOG_INFO
    << conn->peerAddress().toIpPort() << " -> "
    << conn->localAddress().toIpPort() << " is "
    << (conn->connected() ? "UP" : "DOWN");
  {
    MutexLockGuard lck(_mtx);
    if (conn->connected()) {
      _conn = conn;
    } else {
      _conn.reset();
    }
  }
  login(_fmc._username, _fmc._password);
}

void MessageClient::logup(Ctx ctx) {
  printf("logup %s\n", ctx.message.c_str());
}

void MessageClient::login(const std::string& username, const std::string& password) {
  send("/login", username + "\n" + password);
}

void MessageClient::login(Ctx ctx) {
  printf("login %s\n", ctx.message.c_str());
  if (ctx.message != "success") {
    LOG_ERROR << "login";
  }
  auto files = FileModel::scanfPath(_fmc._sharedDirPath);
  auto filesString = FileModel::fileMapToString(files);
  send("/check", filesString);
}
