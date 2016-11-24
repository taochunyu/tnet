#ifndef MESSAGECLIENT_H
#define MESSAGECLIENT_H

#include "tnet.h"
#include "FileModel.h"
class MessageClient : tnet::nocopyable {
 public:
  MessageClient(EventLoop* loop, InetAddress serverAddr, FileModelClient& fmc);
  void connect() { _client.connect(); }
  void send(std::string method, std::string message);
 private:
  void handleConn(const TcpConnectionPtr& conn);
  void login(const std::string& username, const std::string& password);
  void login(Ctx ctx);
  void logup(Ctx ctx);

  EventLoop*       _loop;
  TcpClient        _client;
  Dispather        _dispather;
  Codec            _codec;
  TcpConnectionPtr _conn;
  std::string      _username;
  std::string      _password;
  MutexLock        _mtx;
  FileModelClient&       _fmc;
};

#endif  // MESSAGECLIENT_H
