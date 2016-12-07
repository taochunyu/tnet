#include "tnet.h"
#include "WorkerManager.h"
#include "MessageClient.h"
#include <unistd.h>

int g_out;

void console(const char* message) {
  write(g_out, message, strlen(message));
}

int main(int argc, char const *argv[]) {
  g_out = dup(STDOUT_FILENO);
  close(STDOUT_FILENO);
  int nl = open("/dev/null", O_WRONLY, 0700);
  dup(nl);
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
