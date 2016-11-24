#include "FileServer.h"

FileServer::FileServer(EventLoop* loop, InetAddress listenAddr, FileModelServer& fms)
    : _loop(loop),
      _server(_loop, listenAddr, "FileServer"),
      _fms(fms)
{

}

void FileServer::start(int numThread) {
  assert(numThread > 0);
  _server.setThreadNum(numThread);
  _server.start();
}

void FileServer::trans(const std::string& path) {

}
