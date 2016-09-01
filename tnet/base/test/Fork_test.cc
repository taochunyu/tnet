#include <tnet/base/CurrentThread.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

namespace {
  thread_local int x = 0;
}

void print() {
  printf("pid = %d, tid = %d, x = %d\n", ::getpid(), tnet::CurrentThread::tid(), x);
}

int main() {
  printf("parent %d\n", ::getpid());
  print();
  x = 1;
  print();
  pid_t p = fork();
  if (p == 0) {
    printf("child %d\n", ::getpid());
    print();
    x = 2;
    print();
    if (fork() == 0) {
      printf("grandchild %d\n", ::getpid());
      print();
      x = 3;
      print();
    }
  } else {
    print();
  }
  return 0;
}
