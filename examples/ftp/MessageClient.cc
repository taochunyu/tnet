#include "MessageClient.h"
#include <sstream>

namespace {
  std::pair<std::string, Task> decodeIpTask(std::string& message) {
    std::istringstream is(message);
    std::string ipPort, action, from, to, name;
    is >> ipPort >> action >> from >> to >> name;
    Task task(action, from, to, name);
    return std::make_pair(ipPort, task);
  }

  std::string encodeIpTask(std::string& ipPort, Task& task) {
    std::ostringstream os;
    os
      << ipPort << "\n" << task.action << "\n"
      << task.from << "\n" << task.to << "\n"
      << task.name << "\n";
    return os.str();
  }

}

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
  auto allWorkers = _workers.callTogether();
  for (auto& it : allWorkers) {
    if (_tasks.size() == 0) break;
    auto task = getOneTask();
    auto ipPort = _workers.askWorkerToDo(it, task);
    auto mess = encodeIpTask(ipPort, task);
    send("/newTask", mess);
  }
}

WorkerManager::Worker MessageClient::handleFinishedTask(Task task) {
  auto worker = _workers.whoOwnThisTask(task);
  if (task.action == "loadToClient") {
    close(worker->_currentTaskFd);
    _fmc.lockLink(worker->_currentTask.to, worker->_currentTask.name);
  }
  return worker;
  // else 发送文件的文件描述符在写完成时自动关闭
  // 拓展问题，所有临时文件的文件描述符 应该 在使用完成后自动关闭，并且删除自己，类似于raii，怎么实现？
  // 不是很好实现
}

void MessageClient::loginSuccessCallback() {
  printf("登录成功\n");
  auto files = FileModel::scanfPath(_fmc._sharedDirPath);
  auto filesString = FileModel::fileMapToString(files);
  printf("开始与服务器核对需要同步的文件\n");
  send("/check", filesString);
}

void MessageClient::loginFailureCallback() {
  printf("用户名与密码不符，尝试运行fileSyncConfig重新认证用户信息\n");
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
  printf("系统发现你为新用户，注册成功\n");
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
  const std::string toServer("loadToServer");
  const std::string toClient("loadToClient");
  size_t numOfLoadToClient, numOfLoadToServer;
  std::string from, to, name;

  std::istringstream is(ctx.message);
  is >> numOfLoadToClient >> numOfLoadToServer;
  for (size_t i = 0; i < numOfLoadToClient; i++) {
    is >> from >> name;
    to = _fmc.createTempFileForReceive(name);
    _tasks.emplace_back(toClient, from, to, name);
  }
  for (size_t i = 0; i < numOfLoadToServer; i++) {
    is >> name >> to;
    from = _fmc.createTempFileForSend(name, name);
    _tasks.emplace_back(toServer, from, to, name);
  }
  handleTasks();
}

void MessageClient::ready(Ctx ctx) {
  auto task = decodeIpTask(ctx.message).second;
  auto work = _workers.whoOwnThisTask(task);
  auto mess = ctx.message;
  work->sendFile([this, mess] {
    send("/finishcts", mess);
  });
}

void MessageClient::finish(Ctx ctx) {
  auto task = decodeIpTask(ctx.message).second;
  auto worker = handleFinishedTask(task);
  // next task
  if (_tasks.size() == 0) {
    LOG_INFO << "Finish all tasks";
    return;
  }
  auto newTask = getOneTask();
  auto ipPort = _workers.askWorkerToDo(worker, newTask);
  auto mess = encodeIpTask(ipPort, newTask);
  send("/newTask", mess);
}

// helper
Task MessageClient::getOneTask() {
  MutexLockGuard lck(_tasksMtx);
  assert(_tasks.size() != 0);
  auto temp = _tasks.back();
  _tasks.pop_back();
  return temp;
}
