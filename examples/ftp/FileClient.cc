#include "FileClient.h"
#include <fcntl.h>
#include <unistd.h>

FileClient::FileClient(EventLoop* loop, InetAddress serverAddr, FileModelClient& fmc)
    : _loop(loop),
      _client(_loop, serverAddr, "FileClient"),
      _fmc(fmc),
      _currentTask()
{
  _client.onConnection([this](auto conn){ handleConn(conn); });
  _client.onWriteCompleted([this](auto conn){ onWriteCompleted(conn); });
  _client.onMessage([this](auto conn, auto buf, auto receiveTime){
    onReceiveData(conn, buf, receiveTime);
  });
}

void FileClient::connect() {
  _client.connect();
}

bool FileClient::connected() {
  MutexLockGuard lck(_mtx);
  return _conn != nullptr;
}

std::string FileClient::newTask(Task task) {
  MutexLockGuard lck(_mtx);
  _currentTask = task;
  _currentTaskFinished = false;
  if (task.action == "loadToClient") {
    _currentTaskFd = openat(_fmc._tempDirFd, task.to.c_str(), O_WRONLY, 0700);
    if (_currentTaskFd == -1) {
      LOG_SYSERR << "FileClient::newTask when loadToClient";
    }
  }
  if (task.action == "loadToServer") {
    _currentTaskFd = openat(_fmc._tempDirFd, task.from.c_str(), O_RDONLY, 0700);
    if (_currentTaskFd == -1) {
      LOG_SYSERR << "FileClient::newTask when loadToServer";
    }
  }
  return _conn->localAddress().toIpPort();
}

size_t FileClient::getBytesReceived() {
  size_t temp = _bytesReceived.load();
  _bytesReceived.store(0);
  return temp;
}

void FileClient::sendFile(Callback cb) {
  auto nread = read(_currentTaskFd, _buffer, 65536);
  if (nread == 65536) {
    send(_buffer, 65536);
    _sendFileCallback = cb;
  }
  if (nread > 0 && nread < 65536) {
    send(_buffer, nread);
    _sendFileCallback = cb;
  }
  if (nread == 0) {
    cb();
    close(_currentTaskFd);
    unlinkat(_fmc._tempDirFd, _currentTask.name.c_str(), 0);
  }
  if (nread < 0) {
    LOG_ERROR << "FileClient::sendFile";
    close(_currentTaskFd);
  }
}

void FileClient::send(char* buf, size_t len) {
  if (connected()) {
    _conn->send(buf, len);
  } else {
    LOG_SYSERR << "broken conn";
  }
}

void FileClient::handleConn(const TcpConnectionPtr& conn) {
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
}

void FileClient::onWriteCompleted(const TcpConnectionPtr& conn) {
  auto nread = read(_currentTaskFd, _buffer, 65536);
  if (nread == 65536) {
    send(_buffer, 65536);
  }
  if (nread > 0 && nread < 65536) {
    send(_buffer, nread);
  }
  if (nread == 0) {
    _sendFileCallback();
    close(_currentTaskFd);
    unlinkat(_fmc._tempDirFd, _currentTask.name.c_str(), 0);
  }
  if (nread < 0) {
    LOG_ERROR << "FileClient::sendFile";
    close(_currentTaskFd);
  }
}

void FileClient::onReceiveData(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime) {
  write(_currentTaskFd, buf->peek(), buf->readableBytes());
}
