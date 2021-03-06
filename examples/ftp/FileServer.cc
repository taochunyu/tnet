#include "FileServer.h"
#include <fcntl.h>

FileServer::FileServer(EventLoop* loop, InetAddress listenAddr, FileModelServer& fms)
    : _loop(loop),
      _server(_loop, listenAddr, "FileServer"),
      _fms(fms)
{
  _server.onConnection([this](auto conn){ handleConn(conn); });
  _server.onWriteCompleted([this](auto conn){ onWriteCompleted(conn); });
  _server.onMessage([this](auto conn, auto buf, auto receiveTime){
    onReceiveData(conn, buf, receiveTime);
  });
}

void FileServer::start(int numThread) {
  _server.setThreadNum(numThread);
  _server.start();
}

const TcpConnectionPtr FileServer::newTask(std::string ipPort, Task task, Callback cb) {
  auto it = _ipConnMap.find(ipPort);
  printf("ip地址 %s\n", ipPort.c_str());
  if ( it == _ipConnMap.end()) return nullptr;
  auto currentTask = task;
  auto currentTaskFinished = false;
  int currentTaskFd;
  off_t currentTaskSize = -1;
  printf("新任务%s %s %s %s %s %s\n", task.action.c_str(), task.from.c_str(), task.to.c_str(), task.name.c_str(), task.size.c_str(), task.create.c_str());
  if (task.action == "loadToServer") {
    currentTaskFd = openat(_fms._tempDirFd, task.to.c_str(), O_WRONLY, 0700);
    currentTaskSize = std::stoll(task.size);
  } else {
    currentTaskFd = openat(_fms._tempDirFd, task.from.c_str(), O_RDONLY, 0700);
  }
  Context current = { currentTask, currentTaskFd, currentTaskFinished, currentTaskSize, cb };
  it->second->ctx(current);
  return it->second;
}

void FileServer::handleConn(const TcpConnectionPtr& conn) {
  LOG_INFO
    << conn->peerAddress().toIpPort() << " -> "
    << conn->localAddress().toIpPort() << " is "
    << (conn->connected() ? "UP" : "DOWN");
  if (conn->connected()) {
    MutexLockGuard lck(_mtx);
    _ipConnMap.emplace(conn->peerAddress().toIpPort(), conn);
  } else {
    auto it = _ipConnMap.find(conn->peerAddress().toIpPort());
    assert(it != _ipConnMap.end());
    _ipConnMap.erase(it);
  }
}

void FileServer::onWriteCompleted(const TcpConnectionPtr& conn) {
  printf("写完成\n");
  auto ctx = any_cast<Context>(conn->ctx());
  auto nread = read(ctx.currentTaskFd, ctx.buffer, 65536);
  if (nread == 65536) {
    send(conn, ctx.buffer, 65536);
  }
  if (nread > 0 && nread < 65536) {
    send(conn, ctx.buffer, nread);
  }
  if (nread == 0) {
    ctx.loadToClientFin();
    close(ctx.currentTaskFd);
    unlinkat(_fms._tempDirFd, ctx.currentTask.name.c_str(), 0);
  }
  if (nread < 0) {
    LOG_ERROR << "FileServer::sendFile";
    close(ctx.currentTaskFd);
  }
}

void FileServer::onReceiveData(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime) {
  Context ctx = any_cast<Context>(conn->ctx());
  int nwrite = write(ctx.currentTaskFd, buf->peek(), buf->readableBytes());
  buf->retrieveAll();
  off_t res = lseek(ctx.currentTaskFd, 0, SEEK_CUR);
  if (res == -1) {
    LOG_SYSERR << "lseek";
  }
  printf("完成情况：%d %lld %lld\n", nwrite, res, ctx.currentTaskSize);
  if (res >= ctx.currentTaskSize) {
    ctx.currentTaskFinished = true;
    printf("硬连接 %s %s\n", ctx.currentTask.to.c_str(), ctx.currentTask.name.c_str());
    _fms.lockLink(ctx.currentTask.to, ctx.currentTask.name, ctx.currentTask.create);
    ctx.loadToServerFin();
  }
}

void FileServer::sendFile(const TcpConnectionPtr &conn, Callback cb) {
  auto ctx = any_cast<Context>(conn->ctx());
  auto nread = read(ctx.currentTaskFd, ctx.buffer, 65536);
  printf("发送文件%d, 大小%lu\n", ctx.currentTaskFd, nread);
  if (nread == 65536) {
    send(conn, ctx.buffer, 65536);
    ctx.loadToClientFin = cb;
    conn->ctx(ctx);
  }
  if (nread > 0 && nread < 65536) {
    send(conn, ctx.buffer, nread);
    ctx.loadToClientFin = cb;
    conn->ctx(ctx);
  }
  if (nread == 0) {
    cb();
    close(ctx.currentTaskFd);
    unlinkat(_fms._tempDirFd, ctx.currentTask.name.c_str(), 0);
  }
  if (nread < 0) {
    LOG_ERROR << "FileServer::sendFile";
    close(ctx.currentTaskFd);
  }
  printf("发送文件%d, 大小%lu\n", ctx.currentTaskFd, nread);
}

void FileServer::send(const TcpConnectionPtr& conn, char* buf, size_t len) {
  conn->send(buf, len);
}
