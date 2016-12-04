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
  _dispather.route("/finishcts", [this](auto ctx) { printf("事件 finish\n");finishcts(ctx); });
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
  std::string ipPortStr = ctx.conn->peerAddress().toIpPort();
  std::ostringstream os;
  os << ret.first.size() << "\n" << ret.second.size() << "\n";
  printf("load to client: ");
  printf("容器大小：%lu %lu\n", ret.first.size(), ret.second.size());
  for (size_t i = 0; i < ret.first.size(); i++) {
    auto it = ret.first[i];
    printf("循环一次1\n");
    printf("%s ", it.c_str());
    std::string ret = _fms.createTempFileForSend(ipPortStr, it);
    printf("循环一次2\n");
    os << ret << "\n" << it << "\n";
    printf("循环一次3\n");
  }
  printf("\nload to server: ");
  for (size_t i = 0; i < ret.second.size(); i++) {
    auto it = ret.second[i];
    printf("%s \n", it.c_str());
    std::string ret = _fms.createTempFileForReceive(ipPortStr);
    printf("可\n");
    os << it << "\n" << ret << "\n";
  }
  printf("\n");
  LOG_INFO << "开始发送\n";
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
    auto co = ctx.conn;
    auto mess = ctx.message;
    _fileServer.sendFile(conn, [this, co, mess]{
      printf("完成\n");
      send(co, "/finish", mess);
    });
  } else {
    send(ctx.conn, "/ready", ctx.message);
  }
}

void MessageServer::finishcts(Ctx ctx) {
  std::istringstream is(ctx.message);
  std::string ipPort, to, name;
  is >> ipPort;
  is >> to;
  is >> to;
  is >> to;
  is >> name;
  printf("ipPort: %s\n", ipPort.c_str());
  auto it = _fileServer._ipConnMap.find(ipPort);
  assert(it != _fileServer._ipConnMap.end());
  //auto context = any_cast<FileServer::Context>(it->second->ctx());
  printf("peeraddr: %s\n", it->second->peerAddress().toIpPort().c_str());
  //close(context.currentTaskFd);
  //_fms.lockLink(context.currentTask.to, context.currentTask.name);
  _fms.lockLink(to, name);
  send(ctx.conn, "/finish", ctx.message);
}
