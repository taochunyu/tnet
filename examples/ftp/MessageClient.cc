#include "MessageClient.h"
#include "Console.h"
#include <sstream>

MessageClient::MessageClient(EventLoop* loop, InetAddress listenAddr, FileModelClient& fmc, WorkerManager& wm)
    : _loop(loop),
      _fmc(fmc),
      _client(_loop, listenAddr, "MessageClient"),
      _dispather("MessageDispather"),
      _codec(_dispather.watcher()),
      _workers(wm)
{
  _client.onConnection([this](auto conn){ handleConn(conn); });
  _client.onMessage([this](auto conn, auto buf, auto receiveTime) {
    _codec.onMessage(conn, buf, receiveTime);
  });
  _dispather.route("/logup", [this](auto ctx){ printf("事件 logup\n");logup(ctx); });
  _dispather.route("/login", [this](auto ctx){ printf("事件 login\n");login(ctx); });
  _dispather.route("/tasks", [this](auto ctx){ printf("事件 tasks\n");tasks(ctx); });
  _dispather.route("/ready", [this](auto ctx){ printf("事件 ready\n");ready(ctx); });
  _dispather.route("/finish", [this](auto ctx){ printf("事件 finish\n");finish(ctx); });
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

void MessageClient::login(const std::string& username, const std::string& password) {
  send("/login", username + "\n" + password);
}

void MessageClient::handleTasks() {
  if (_tasks.size() == 0) {
    LOG_INFO << "nothing to do";
  }
  _workers.setSender([this](auto a, auto b) { send(a, b); });
  _workers.doThoseTasks(_tasks);
}

void MessageClient::loginSuccessCallback() {
  console("登录成功\n");
  auto files = FileModel::scanfPath(_fmc._sharedDirPath);
  auto filesString = FileModel::fileMapToString(files);
  console("开始与服务器核对需要同步的文件\n");
  send("/check", filesString);
}

void MessageClient::loginFailureCallback() {
  console("用户名与密码不符，尝试运行fileSyncConfig重新认证用户信息\n");
  quit();
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
  console("系统发现你为新用户，注册成功\n");
  loginSuccessCallback();
}

void MessageClient::login(Ctx ctx) {
  if (ctx.message == "success") {
    loginSuccessCallback();
  } else {
    loginFailureCallback();
  }
}

void MessageClient::tasks(Ctx ctx) {
  console("核对完成\n");
  const std::string toServer("loadToServer");
  const std::string toClient("loadToClient");
  size_t numOfLoadToClient, numOfLoadToServer;
  std::string from, to, name, size, create;

  std::istringstream is(ctx.message);
  is >> numOfLoadToClient >> numOfLoadToServer;
  for (size_t i = 0; i < numOfLoadToClient; i++) {
    is >> from >> name >> create >> size;
    to = _fmc.createTempFileForReceive(name);
    _tasks.emplace_back(toClient, from, to, name, size, create);
  }
  for (size_t i = 0; i < numOfLoadToServer; i++) {
    is >> name >> create >> size >> to;
    from = _fmc.createTempFileForSend(name, name);
    LOG_INFO << from << to << name << size << create;
    _tasks.emplace_back(toServer, from, to, name, size, create);
  }
  handleTasks();
}

void MessageClient::ready(Ctx ctx) {
  auto task = decodeIpTask(ctx.message).second;
  auto mess = ctx.message;
  _workers.ready(task, [this, mess] {
    printf("完成\n");
  });
}

void MessageClient::finish(Ctx ctx) {
  auto task = decodeIpTask(ctx.message).second;
  _workers.nextTask(task);
}
