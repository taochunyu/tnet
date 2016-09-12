#include <tnet/base/nocopyable.h>

class Test : tnet::nocopyable {};

int main() {
  // Test test1;
  // Test test2(test1);
  // compile error :
  // call to implicitly-deleted copy constructor of 'Test'
  // Test test3;
  // test3 = test1;
  // compile error :
  // object of type 'Test' cannot be assigned
  // because its copy assigment operator is implicitly deleted
  return 0;
}
