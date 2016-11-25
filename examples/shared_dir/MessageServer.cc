#include "MessageServer.h"
#include <sstream>



MessageServer::MessageServer(EventLoop* loop, InetAddress listenAddr, FileModelServer& fms)
    : _loop(loop),
      _server(_loop, listenAddr, "MessageServer"),
      _dispather("MessageDispather"),
      _codec(_dispather.watcher()),
      _fms(fms)
{
  _server.onConnection([this](auto conn){ handleConn(conn); });
  _server.onMessage([this](auto conn, auto buf, auto receiveTime) {
    _codec.onMessage(conn, buf, receiveTime);
  });
  _dispather.route("/login", [this](auto ctx) { login(ctx); });
  _dispather.route("/check", [this](auto ctx) { check(ctx); });
  _dispather.configDone();
}

void MessageServer::handleConn(const TcpConnectionPtr &conn) {
  LOG_INFO
    << conn->peerAddress().toIpPort() << " -> "
    << conn->localAddress().toIpPort() << " is "
    << (conn->connected() ? "UP" : "DOWN");
}

void MessageServer::send(const TcpConnectionPtr& conn, std::string method, std::string message) {
  _codec.send(conn, method, message);
}

void MessageServer::login(Ctx ctx) {
  std::istringstream is(ctx.message);
  std::string username, password;
  is >> username;
  is >> password;
  auto where = _fms._usersList.find(username);
  if (where == _fms._usersList.end()) {
    _fms._usersList.emplace(username, password);
    send(ctx.conn, "/logup", "success");
  } else {
    if (where->second == password) {
      send(ctx.conn, "/login", "success");
    } else {
      send(ctx.conn, "/login", "wrong password");
    }
  }
}

void MessageServer::check(Ctx ctx) {
  auto clientFiles = FileModel::stringToFileMap(ctx.message);
  auto serverFiles = FileModel::scanfPath(_fms._sharedDirPath);
  auto ret = FileModel::fileMapCmper(clientFiles, serverFiles);
  printf("load to client: ");
  for (auto it : ret.first) {
    printf("%s ", it.c_str());
  }
  printf("\nload to server: ");
  for (auto it : ret.second) {
    printf("%s ", it.c_str());
  }
  printf("\n");
}