#ifndef FILECLIENT_H
#define FILECLIENT_H

#include "tnet.h"
#include "FileModel.h"

class FileClient : tnet::nocopyable {
  friend class MessageClient;
 public:
  using Job = struct {
    std::string action;
    std::string from;
    std::string to;
  };
  static const int kPackageSize = 65536;
  FileClient(EventLoop* loop, InetAddress serverAddr, FileModelClient& fmc);
  void connect() {_client.connect(); }
  void doJob(Job job);   // block function
 private:
  void handleConn(const TcpConnectionPtr&);
  void whenWriteComplete(const TcpConnectionPtr&);
  void whenReceiveData(const TcpConnectionPtr&, Buffer*, Timestamp);
  EventLoop* _loop;
  TcpClient _client;
  FileModelClient& _fmc;
  TcpConnectionPtr _conn;
  MutexLock _mtx;
  Job _currentJob;
  int _currentJobFd;
  bool _currentJobFinished;
};

#endif  //FILECLIENT_H
