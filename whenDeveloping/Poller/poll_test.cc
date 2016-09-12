#include <tnet/base/Logging.h>
#include <tnet/base/Exception.h>
#include <iostream>

int main() {
  std::cout << "hi world" << std::endl;
  LOG_TRACE << "hello world";
  return 0;
}
