#include <gtest/gtest.h>
#include <tnet/base/Exception.h>
#include <cstdio>

class ExceptionTest : public testing::Test{
 protected:
  virtual void SetUp() {}
  virtual void TearDown() {}
  class Bar {
   public:
    void test() {
      throw tnet::Exception("oops");
    }
  };
  void foo() {
    Bar b;
    b.test();
  }
};

TEST_F(ExceptionTest, tryCatchTest) {
  try {
    foo();
  } catch(const tnet::Exception &ex) {
    EXPECT_STREQ("oops", ex.what());
    printf("stack trace: %s\n", ex.stackTrace());
  }
}
