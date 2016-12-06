#include "tnet.h"
#include "WorkerManager.h"
#include "MessageClient.h"

int main(int argc, char const *argv[]) {
  EventLoop loop;
  FileModelClient fileModelClient;
  fileModelClient.readConfigFile();
  InetAddress messServerAddr("127.0.0.1", 8080);
  InetAddress fileServerAddr("127.0.0.1", 8081);

  WorkerManager workers(&loop, fileServerAddr, fileModelClient, 2);
  MessageClient messClient(&loop, messServerAddr, fileModelClient, workers);
  messClient.connect();
  loop.loop();
  return 0;
}
