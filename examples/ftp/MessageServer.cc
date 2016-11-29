#include "MessageServer.h"
#include <sstream>

MessageServer::MessageServer(EventLoop* loop, InetAddress listenAddr, FileModelServer& fms, FileServer& fs)
    : _loop(loop),
      _server(_loop, listenAddr, "MessageServer"),
      _dispather("MessageDispather"),
      _codec(_dispather.watcher()),
      _fms(fms),
      _fileServer(fs)
{
  _server.onConnection([this](auto conn){ handleConn(conn); });
  _server.onMessage([this](auto conn, auto buf, auto receiveTime) {
    _codec.onMessage(conn, buf, receiveTime);
  });
  _dispather.route("/login", [this](auto ctx) { printf("事件 login\n"); login(ctx); });
  _dispather.route("/check", [this](auto ctx) { printf("事件 check\n");check(ctx); });
  _dispather.route("/newTask", [this](auto ctx) { printf("事件 newTask\n");newTask(ctx); });
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
  printf("啦啦啦\n");
  auto clientFiles = FileModel::stringToFileMap(ctx.message);
  auto serverFiles = FileModel::scanfPath(_fms._sharedDirPath);
  auto ret = FileModel::fileMapCmper(clientFiles, serverFiles);
  std::string ipPortStr = ctx.conn->peerAddress().toIpPort();
  std::ostringstream os;
  printf("load to client: ");
  for (auto it : ret.first) {
    printf("%s ", it.c_str());
    std::string ret = _fms.createTempFileForSend(ipPortStr, it);
    os << ret << "\n" << it << "\n";
  }
  os << "####" << "\n";
  printf("\nload to server: ");

  for (auto it : ret.second) {
    printf("%s ", it.c_str());
    std::string ret = _fms.createTempFileForReceive(ipPortStr);
    os << it << "\n" << ret << "\n";
  }
  printf("\n");
  send(ctx.conn, "/tasks", os.str());
}

void MessageServer::newTask(Ctx ctx) {
  std::istringstream is(ctx.message);
  std::string ipPort, action, from, to, name;
  is >> ipPort;
  is >> action;
  is >> from;
  is >> to;
  is >> name;
  Task task(action, from, to, name);
  MutexLockGuard lck(_mtx);
  _taskConnMap.emplace(task, ctx.conn);
  auto conn = _fileServer.newTask(ipPort, task);
  if (!conn) return;
  if (action == "loadToClient") {
    _fileServer.sendFile(conn, [this, ctx]{
      send(ctx.conn, "/finish", ctx.message);
    });
  } else {
    send(ctx.conn, "/ready", ctx.message);
  }
}

void MessageServer::finish(Ctx ctx) {
  auto context = any_cast<Context>(ctx.conn->ctx());
  close(context.currentTaskFd);
  _fms.lockLink(context.currentTask.to, context.currentTask.name);
  send(ctx.conn, "/finish", ctx.message);
}
