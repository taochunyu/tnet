#include "MessageServer.h"

int main(int argc, char const *argv[]) {
  FileModel fm;
  EventLoop loop;
  InetAddress messListenAddr(8080);
  MessageServer messServer(&loop, messListenAddr, fm);
  messServer.start();
  loop.loop();
  return 0;
}
