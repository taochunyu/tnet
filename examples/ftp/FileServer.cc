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

const TcpConnectionPtr FileServer::newTask(std::string ipPort, Task task) {
  auto it = _ipConnMap.find(ipPort);
  if ( it == _ipConnMap.end()) return nullptr;
  auto currentTask = task;
  auto currentTaskFinished = false;
  int currentTaskFd;
  if (task.action == "loadToClient") {
    currentTaskFd = openat(_fms._tempDirFd, task.to.c_str(), O_WRONLY | O_TRUNC, 0700);
  } else {
    currentTaskFd = openat(_fms._tempDirFd, task.from.c_str(), O_RDONLY, 0700);
  }
  Context current = { currentTask, currentTaskFd, currentTaskFinished};
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
  auto ctx = any_cast<Context>(conn->ctx());
  auto nread = read(ctx.currentTaskFd, ctx.buffer, 65536);
  if (nread == 65536) {
    send(conn, ctx.buffer, 65536);
  }
  if (nread > 0 && nread < 65536) {
    send(conn, ctx.buffer, nread);
  }
  if (nread == 0) {
    ctx.callback();
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
  write(ctx.currentTaskFd, buf->peek(), buf->readableBytes());
}

void FileServer::sendFile(const TcpConnectionPtr &conn, Callback cb) {
  auto ctx = any_cast<Context>(conn->ctx());
  auto nread = read(ctx.currentTaskFd, ctx.buffer, 65536);
  if (nread == 65536) {
    send(conn, ctx.buffer, 65536);
    ctx.callback = cb;
    conn->ctx(ctx);
  }
  if (nread > 0 && nread < 65536) {
    send(conn, ctx.buffer, nread);
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
}

void FileServer::send(const TcpConnectionPtr& conn, char* buf, size_t len) {
  conn->send(buf, len);
}
