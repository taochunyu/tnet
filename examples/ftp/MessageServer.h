#ifndef MESSAGESERVER_H
#define MESSAGESERVER_H

#include "tnet.h"
#include "Task.h"
#include "FileModel.h"
#include "FileServer.h"

class MessageServer : tnet::nocopyable {
 public:
  MessageServer(EventLoop* loop, InetAddress listenAddr, FileModelServer& fms, FileServer& fs);
  void start() { _server.start(); }
 private:
  void loadToClient(std::string ipPort, Task task);

  void handleConn(const TcpConnectionPtr& conn);
  void send(const TcpConnectionPtr& conn, std::string method, std::string message);
  void login(Ctx ctx);
  void check(Ctx ctx);
  void newTask(Ctx ctx);
  void finish(Ctx ctx);

  EventLoop*        _loop;
  TcpServer         _server;
  Dispather         _dispather;
  Codec             _codec;
  FileModelServer&  _fms;
  FileServer&       _fileServer;

  std::map<Task, TcpConnectionPtr> _taskConnMap;
  MutexLock                        _mtx;
};

#endif  // MESSAGESERVER_H
