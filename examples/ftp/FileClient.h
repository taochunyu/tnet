#ifndef FILECLIENT_H
#define FILECLIENT_H

#include "tnet.h"
#include "Task.h"
#include "FileModel.h"

class FileClient : tnet::nocopyable {
  friend class MessageClient;
 public:
  FileClient(EventLoop* loop, InetAddress serverAddr, FileModelClient& fmc);
  void connect();
  bool connected();
  std::string newTask(Task task);
  size_t getBytesReceived();
  void sendFile(Callback cb);
  void send(char* buf, size_t len);
 private:
  void handleConn(const TcpConnectionPtr&);
  void onWriteCompleted(const TcpConnectionPtr&);
  void onReceiveData(const TcpConnectionPtr&, Buffer*, Timestamp);

  EventLoop* _loop;
  TcpClient _client;
  FileModelClient& _fmc;
  TcpConnectionPtr _conn;
  MutexLock _mtx;
  Task _currentTask;
  int _currentTaskFd;
  bool _currentTaskFinished;
  char _buffer[65536];
  Callback _sendFileCallback;
  std::atomic<size_t> _bytesReceived;
};

#endif  //FILECLIENT_H
