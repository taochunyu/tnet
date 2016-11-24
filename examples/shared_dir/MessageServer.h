#ifndef MESSAGESERVER_H
#define MESSAGESERVER_H

#include "tnet.h"
#include "FileModel.h"

class MessageServer : tnet::nocopyable {
 public:
  MessageServer(EventLoop* loop, InetAddress listenAddr, FileModelServer& fms);
  void start() { _server.start(); }
 private:
  static void loadToClient(const std::string& path);
  static void loadToServer(const std::string& path);
  void handleConn(const TcpConnectionPtr& conn);
  void send(const TcpConnectionPtr& conn, std::string method, std::string message);
  void login(Ctx ctx);
  void check(Ctx ctx);
  EventLoop*        _loop;
  TcpServer         _server;
  Dispather         _dispather;
  Codec             _codec;
  FileModelServer&  _fms;
};

#endif  // MESSAGESERVER_H
