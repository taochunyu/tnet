#ifndef FILESERVER_H
#define FILESERVER_H

#include "tnet.h"
#include "FileModel.h"

class FileServer : tnet::nocopyable {
 public:
  FileServer(EventLoop* loop, InetAddress listenAddr, FileModelServer& fms);
  void start() { _server.start(); }
  void start(int numThread);
  void loadToServer(const std::string& path);
  void loadToClient(const std::string& path);
 private:
  EventLoop*      _loop;
  TcpServer       _server;
  FileModelServer& _fms;
};

#endif  // FILESERVER_H
