#include <gtest/gtest.h>
#include <tnet/base/Timestamp.h>
#include <cstdio>
#include <cstring>

class TimestampTest : public testing::Test {
 protected:
  virtual void SetUp() {}
  virtual void TearDown() {}
  void passByConstRef(const tnet::Timestamp &stamp) {
    sprintf(buffer, "%s", stamp.toString().c_str());
  }
  void passByValue(tnet::Timestamp stamp) {
    sprintf(buffer, "%s", stamp.toString().c_str());
  }
  char buffer[20];
};

TEST_F(TimestampTest, PassTest) {
  char now[20];
  tnet::Timestamp stamp(tnet::Timestamp::now());
  sprintf(now, "%s", stamp.toString().c_str());
  passByConstRef(stamp);
  EXPECT_STREQ(buffer, now);
  passByValue(stamp);
  EXPECT_STREQ(buffer, now);
}

#ifndef GTEST_ALL
int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
#endif
