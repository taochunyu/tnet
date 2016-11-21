#ifndef MESSAGECLIENT_H
#define MESSAGECLIENT_H

#include "tnet.h"

class FileModel;
class MessageClient : tnet::nocopyable {
 public:
  MessageClient(EventLoop* loop, InetAddress serverAddr, FileModel& fm);
  void connect() { _client.connect(); }
  void send(std::string method, std::string message);
 private:
  void handleConn(const TcpConnectionPtr& conn);
  void logup(Ctx ctx);

  EventLoop*       _loop;
  TcpClient        _client;
  Dispather        _dispather;
  Codec            _codec;
  TcpConnectionPtr _conn;
  std::string      _username;
  std::string      _password;
  MutexLock        _mtx;
  FileModel&       _fm;
};

#endif  // MESSAGECLIENT_H
