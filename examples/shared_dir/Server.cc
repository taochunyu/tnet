#include "MessageServer.h"

int main(int argc, char const *argv[]) {
  FileModelServer fms;
  fms.readConfigFile();
  EventLoop loop;
  InetAddress messListenAddr(8080);
  MessageServer messServer(&loop, messListenAddr, fms);
  messServer.start();
  loop.loop();
  return 0;
}
