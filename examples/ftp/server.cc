#include "tnet.h"
#include "FileModel.h"
#include "FileServer.h"
#include "MessageServer.h"

int main(int argc, char const *argv[]) {
  FileModelServer fileModelServer;
  fileModelServer.readConfigFile();
  EventLoop loop;
  InetAddress messListenAddr(8080);
  InetAddress fileListenAddr(8081);

  FileServer fileServer(&loop, fileListenAddr, fileModelServer);
  MessageServer messServer(&loop, messListenAddr, fileModelServer, fileServer);
  fileServer.start(0);
  messServer.start();
  loop.loop();
  return 0;
}
