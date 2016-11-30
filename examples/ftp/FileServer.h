#ifndef FILESERVER_H
#define FILESERVER_H

#include "tnet.h"
#include "Task.h"
#include "FileModel.h"

namespace {

struct Context {
  Task currentTask;
  int  currentTaskFd;
  bool currentTaskFinished;
  char buffer[65535];
  Callback callback;
};

}

class FileServer : tnet::nocopyable {
  friend class MessageServer;
 public:
  using Context = struct Context;
  FileServer(EventLoop* loop, InetAddress listenAddr, FileModelServer& fms);
  void start(int numThread = 1);
  const TcpConnectionPtr newTask(std::string ipPort, Task task);
  void sendFile(const TcpConnectionPtr& conn, Callback cb);
  void send(const TcpConnectionPtr& conn, char* buf, size_t len);
 private:
  void handleConn(const TcpConnectionPtr&);
  void onWriteCompleted(const TcpConnectionPtr&);
  void onReceiveData(const TcpConnectionPtr&, Buffer*, Timestamp);
  EventLoop* _loop;
  TcpServer  _server;
  FileModelServer& _fms;
  std::map<std::string, TcpConnectionPtr> _ipConnMap;
  MutexLock                               _mtx;
};

#endif  // FILESERVER_H
