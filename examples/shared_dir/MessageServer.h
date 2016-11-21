#ifndef MESSAGESERVER_H
#define MESSAGESERVER_H

#include "tnet.h"

class FileModel;
class MessageServer : tnet::nocopyable {
 public:
  MessageServer(EventLoop* loop, InetAddress listenAddr, FileModel& fm);
  void start() { _server.start(); }
 private:
  void handleConn(const TcpConnectionPtr& conn);
  void send(const TcpConnectionPtr& conn, std::string method, std::string message);
  void logup(Ctx ctx);
  void login(Ctx ctx);
  void checkState(Ctx ctx);
  EventLoop* _loop;
  TcpServer  _server;
  Dispather  _dispather;
  Codec      _codec;
  FileModel&  _fm;
};

#endif  // MESSAGESERVER_H
