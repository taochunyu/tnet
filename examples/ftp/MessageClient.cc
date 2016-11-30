#include "MessageClient.h"
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

void MessageClient::tasks(Ctx ctx) {
  std::istringstream is(ctx.message);
  std::string from, to, name;
  std::string toServer("loadToServer");
  std::string toClient("loadToClient");
  while (is >> from) {
    if (from == "####") break;
    is >> to;
    name = to;
    to = _fmc.createTempFileForReceive(to);
    _tasks.emplace_back(toClient, from, to, name);
  }
  while (is >> from) {
    is >> to;
    name = from;
    from = _fmc.createTempFileForSend(from, from);
    _tasks.emplace_back(toServer, from, to, name);
  }
  handleTasks();
}

void MessageClient::handleTasks() {
  printf("开始处理事件0\n");
  MutexLockGuard lck(_tasksMtx);
  auto allWorkers = _workers.callTogether();
  printf("%lu\n", allWorkers.size());
  if (_tasks.size() == 0) {
    LOG_INFO << "nothing to do";
  }
  printf("开始处理事件1\n");
  for (auto& it : allWorkers) {
    printf("开始处理事件2\n");

    if (_tasks.size() == 0) break;
    printf("开始处理事件3\n");
    auto task = _tasks.back();
    printf("开始处理事件4\n");
    auto ipPort = _workers.askWorkerToDo(it, task);
    printf("开始处理事件5\n");
    std::ostringstream os;
    os
      << ipPort << "\n" << task.action << "\n"
      << task.from << "\n" << task.to << "\n"
      << task.name << "\n";
    _tasks.pop_back();
    printf("事件处理中\n");
    send("/newTask", os.str());
  }
}

void MessageClient::ready(Ctx ctx) {
  std::istringstream is(ctx.message);
  std::string action, from, to, name;
  is >> action;  // ipPort
  is >> action;
  is >> from;
  is >> to;
  is >> name;
  Task task(action, from, to, name);
  auto work = _workers.whoOwnThisTask(task);
  auto mess = ctx.message;
  work->sendFile([this, mess] {
    send("/finishcts", mess);
  });
}

void MessageClient::finish(Ctx ctx) {
  MutexLockGuard lck(_tasksMtx);
  std::istringstream is(ctx.message);
  std::string action, from, to, name;
  is >> action;  // ipPort
  is >> action;
  is >> from;
  is >> to;
  is >> name;
  LOG_INFO << "内容" <<ctx.message;
  Task task(action, from, to, name);
  auto worker = _workers.whoOwnThisTask(task);
  close(worker->_currentTaskFd);
  _fmc.lockLink(worker->_currentTask.to, worker->_currentTask.name);
  if (_tasks.size() == 0) {
    LOG_INFO << "Finish all tasks";
    return;
  }
  auto newOne = _tasks.back();
  auto ipPort = _workers.askWorkerToDo(worker, newOne);
  _tasks.pop_back();
  std::ostringstream os;
  os
    << ipPort << "\n" << newOne.action << "\n"
    << newOne.from << "\n" << newOne.to << "\n"
    << newOne.name << "\n";
  send("/newTask", os.str());
}
