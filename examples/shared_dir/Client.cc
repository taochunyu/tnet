#include "MessageClient.h"
#include <iostream>

int main(int argc, char const *argv[]) {
  FileModel fm;
  EventLoopThread loopThread;
  InetAddress messServerAddr("127.0.0.1", 8080);
  MessageClient messClient(loopThread.startLoop(), messServerAddr, fm);
  messClient.connect();
  std::string line;
  while (std::getline(std::cin, line)) {
    messClient.send("/logup", line);
  }
  return 0;
}
