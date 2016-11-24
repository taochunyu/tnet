#include "MessageClient.h"
#include <iostream>

int main(int argc, char const *argv[]) {
  FileModelClient fmc;
  fmc.readConfigFile();
  EventLoop loop;
  InetAddress messServerAddr("127.0.0.1", 8080);
  MessageClient messClient(&loop, messServerAddr, fmc);
  messClient.connect();
  loop.loop();
  return 0;
}
