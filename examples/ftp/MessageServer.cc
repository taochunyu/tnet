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
  std::string name, password;
  is >> name >> password;
  auto it = _fms._usersList.find(name);
  if (it == _fms._usersList.end()) {
    _fms.addUser(name, password);
    send(ctx.conn, "/logup", "success");
  } else {
    if (it->second == password) {
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
  std::string ipPortStr = ctx.conn->peerAddress().toIpPort(), name;
  std::ostringstream os;
  os << ret.first.size() << "\n" << ret.second.size() << "\n";
  printf("load to client: ");
  for (auto it : ret.first) {
    printf("%s ", it.c_str());
    std::istringstream is(it);
    is >> name;
    std::string ret = _fms.createTempFileForSend(ipPortStr, name);
    os << ret << "\n" << it << "\n";
  }
  printf("\nload to server: ");
  for (auto it : ret.second) {
    printf("%s ", it.c_str());
    std::istringstream is(it);
    is >> name;
    std::string ret = _fms.createTempFileForReceive(ipPortStr, name);
    os << it << "\n" << ret << "\n";
  }
  printf("\n");
  LOG_INFO << "开始发送\n";
  send(ctx.conn, "/tasks", os.str());
}

void MessageServer::newTask(Ctx ctx) {
  auto ipTask = decodeIpTask(ctx.message);
  auto ipPort = ipTask.first;
  auto task = ipTask.second;
  printf("检查%s %s %s %s %s %s\n", task.action.c_str(), task.from.c_str(), task.to.c_str(), task.name.c_str(), task.size.c_str(), task.create.c_str());
  MutexLockGuard lck(_mtx);
  _taskConnMap.emplace(task, ctx.conn);
  auto mess = ctx.message;
  auto co = ctx.conn;
  auto conn = _fileServer.newTask(ipPort, task, [this, mess, co] {
    send(co, "/finish", mess);
  });
  if (!conn) return;
  if (task.action == "loadToClient") {
    auto co = ctx.conn;
    auto mess = ctx.message;
    _fileServer.sendFile(conn, []{
      printf("完成\n");
    });
  } else {
    send(ctx.conn, "/ready", ctx.message);
  }
}
