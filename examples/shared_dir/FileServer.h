#ifndef FILESERVER_H
#define FILESERVER_H

#include "tnet.h"
#include "FileModel.h"
#include <tuple>

class FileServer : tnet::nocopyable {
 public:                      // ipPortStr    action       payload
  using Job = std::tuple<std::string, std::string, std::string>;
  FileServer(EventLoop* loop, InetAddress listenAddr, FileModelServer fms);
  void newJob(Job job, Callback cb);
 private:
  EventLoop* loop;
  TcpServer  _server;
  FileModelServer _fms;
  std::map<std::string, TcpConnectionPtr> ipConnMapping;
};

#endif  // FILESERVER_H
