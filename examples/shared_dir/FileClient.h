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
  void stateTrans(Job job);
  size_t getBytesReceived() {
    size_t temp = _bytesReceived.load();
    _bytesReceived.store(0);
    return temp;
  }
 private:
  void handleConn(const TcpConnectionPtr&);
  void onWriteCompleted(const TcpConnectionPtr&);
  void onReceiveData(const TcpConnectionPtr&, Buffer*, Timestamp);
  void onFinished();
  EventLoop* _loop;
  TcpClient _client;
  FileModelClient& _fmc;
  TcpConnectionPtr _conn;
  MutexLock _mtx;
  Job _currentJob;
  int _currentJobFd = -1;
  bool _currentJobFinished;
  std::atomic<size_t> _bytesReceived;
};

#endif  //FILECLIENT_H
