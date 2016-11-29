#include "FileClient.h"
#include <fcntl.h>
#include <unistd.h>

FileClient::FileClient(EventLoop* loop, InetAddress serverAddr, FileModelClient& fmc)
    : _loop(loop),
      _client(_loop, serverAddr, "FileClient"),
      _fmc(fmc)
{
  _client.onConnection([this](auto conn){ handleConn(conn); });
  _client.onWriteCompleted([this](auto conn){ onWriteCompleted(conn); });
  _client.onMessage([this](auto conn, auto buf, auto receiveTime){
    onReceiveData(conn, buf, receiveTime);
  });
}

void FileClient::stateTrans(Job job) {
  MutexLockGuard lck(_mtx);
  _currentJob = job;
  _currentJobFinished = false;
  if (job.action == "loadToClient") {
    _currentJobFd = openat(_fmc._tempDirFd, job.to.c_str(), O_WRONLY | O_TRUNC, 0700);
    if (_currentJobFd == -1) {
      LOG_SYSERR << "FileClient::stateTrans when loadToClient";
    }
  }
  if (job.action == "loadToServer") {
    _currentJobFd = openat(_fmc._tempDirFd, job.from.c_str(), O_RDONLY, 0700);
    if (_currentJobFd == -1) {
      LOG_SYSERR << "FileClient::stateTrans when loadToServer";
    }
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

}

void FileClient::onReceiveData(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime) {
  write(_currentJobFd, buf->peek(), buf->readableBytes());
}
