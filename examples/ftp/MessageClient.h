#ifndef MESSAGECLIENT_H
#define MESSAGECLIENT_H

#include "tnet.h"
#include "FileModel.h"
#include "WorkerManager.h"

class MessageClient : tnet::nocopyable {
 public:
  MessageClient(EventLoop* loop, InetAddress serverAddr, FileModelClient& fmc, WorkerManager& wm);
  void connect() { _client.connect(); }
  void send(std::string method, std::string message);
  void quit() { _client.getLoop()->quit(); }
 private:

  void login(const std::string& username, const std::string& password);
  void handleTasks();
  WorkerManager::Worker handleFinishedTask(Task task);

  void loginSuccessCallback();
  void loginFailureCallback();

  // controller
  void handleConn(const TcpConnectionPtr& conn);
  void login(Ctx ctx);
  void logup(Ctx ctx);
  void tasks(Ctx ctx);
  void ready(Ctx ctx);
  void finish(Ctx ctx);


  EventLoop*        _loop;
  FileModelClient&  _fmc;
  TcpClient         _client;
  Dispather         _dispather;
  Codec             _codec;
  TcpConnectionPtr  _conn;
  MutexLock         _mtx;
  WorkerManager&    _workers;

  std::string       _username;
  std::string       _password;
  std::vector<Task> _tasks;
};

#endif  // MESSAGECLIENT_H
