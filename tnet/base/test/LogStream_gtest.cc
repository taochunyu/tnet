#include <gtest/gtest.h>
#include <tnet/base/LogStream.h>
#include <string>
#include <limits>

class LogStreamTest : public testing::Test {
 protected:
  virtual void SetUp() {
    EXPECT_EQ(getString(), "");
  }
  virtual void TearDown() {}
  std::string getString() {
    return ls.buffer().toString();
  }
  int getLength() {
    return ls.buffer().length();
  }
  int getAvail() {
    return ls.buffer().avail();
  }
  tnet::LogStream ls;
};

TEST_F(LogStreamTest, BoolTest) {
  ls << true;
  EXPECT_EQ(getString(), "true");
  ls << '\n';
  EXPECT_EQ(getString(), "true\n");
  ls << false;
  EXPECT_EQ(getString(), "true\nfalse");
}

TEST_F(LogStreamTest, IntegerTest) {
  ls << 1;
  EXPECT_EQ(getString(), "1");
  ls << 0;
  EXPECT_EQ(getString(), "10");
  ls << -1;
  EXPECT_EQ(getString(), "10-1");
  ls.resetBuffer();
  ls << 1 << ' ' << "123" << 0x64;
  EXPECT_EQ(getString(), "1 123100");
}

TEST_F(LogStreamTest, LimitsIntegerTest) {
  ls << -2147483647;
  EXPECT_EQ(getString(), "-2147483647");
  ls << static_cast<int>(-2147483647 - 1);
  EXPECT_EQ(getString(), "-2147483647-2147483648");
  ls << ' ';
  ls << 2147483647;
  EXPECT_EQ(getString(), "-2147483647-2147483648 2147483647");
  ls.resetBuffer();

  ls << std::numeric_limits<int16_t>::min();
  EXPECT_EQ(getString(), "-32768");
  ls.resetBuffer();
  ls << std::numeric_limits<int16_t>::max();
  EXPECT_EQ(getString(), "32767");
  ls.resetBuffer();
  ls << std::numeric_limits<uint16_t>::min();
  EXPECT_EQ(getString(), "0");
  ls.resetBuffer();
  ls << std::numeric_limits<uint16_t>::max();
  EXPECT_EQ(getString(), "65535");
  ls.resetBuffer();

  ls << std::numeric_limits<int32_t>::min();
  EXPECT_EQ(getString(), "-2147483648");
  ls.resetBuffer();
  ls << std::numeric_limits<int32_t>::max();
  EXPECT_EQ(getString(), "2147483647");
  ls.resetBuffer();
  ls << std::numeric_limits<uint32_t>::min();
  EXPECT_EQ(getString(), "0");
  ls.resetBuffer();
  ls << std::numeric_limits<uint32_t>::max();
  EXPECT_EQ(getString(), "4294967295");
  ls.resetBuffer();

  ls << std::numeric_limits<int64_t>::min();
  EXPECT_EQ(getString(), "-9223372036854775808");
  ls.resetBuffer();
  ls << std::numeric_limits<int64_t>::max();
  EXPECT_EQ(getString(), "9223372036854775807");
  ls.resetBuffer();
  ls << std::numeric_limits<uint64_t>::min();
  EXPECT_EQ(getString(), "0");
  ls.resetBuffer();
  ls << std::numeric_limits<uint64_t>::max();
  EXPECT_EQ(getString(), "18446744073709551615");
  ls.resetBuffer();

  int16_t a = 0;
  int32_t b = 0;
  int64_t c = 0;
  ls << a << b << c;
  EXPECT_EQ(getString(), "000");
}

TEST_F(LogStreamTest, FloatsTest) {
  ls << 0.0;
  EXPECT_EQ(getString(), "0");
  ls.resetBuffer();
  ls << 1.0;
  EXPECT_EQ(getString(), "1");
  ls.resetBuffer();
  ls << 0.1;
  EXPECT_EQ(getString(), "0.1");
  ls.resetBuffer();
  ls << 0.05;
  EXPECT_EQ(getString(), "0.05");
  ls.resetBuffer();
  ls << 0.15;
  EXPECT_EQ(getString(), "0.15");
  ls.resetBuffer();

  double a = 0.1;
  ls << a;
  EXPECT_EQ(getString(), "0.1");
  ls.resetBuffer();
  double b = 0.05;
  ls << b;
  EXPECT_EQ(getString(), "0.05");
  ls.resetBuffer();
  double c = 0.15;
  ls << c;
  EXPECT_EQ(getString(), "0.15");
  ls.resetBuffer();

  EXPECT_FALSE(a + b == c);

  ls << 1.23456789;
  EXPECT_EQ(getString(), "1.23456789");
  ls.resetBuffer();
  ls << 1.234567;
  EXPECT_EQ(getString(), "1.234567");
  ls.resetBuffer();
  ls << -123.456;
  EXPECT_EQ(getString(), "-123.456");
  ls.resetBuffer();
}

TEST_F(LogStreamTest, VoidTest) {
  ls << static_cast<void*>(0);
  EXPECT_EQ(getString(), std::string("0x0"));
  ls.resetBuffer();
  ls << reinterpret_cast<void*>(8888);
  EXPECT_EQ(getString(), std::string("0x22B8"));
  ls.resetBuffer();
}

TEST_F(LogStreamTest, StringTest) {
  ls << "hello ";
  EXPECT_EQ(getString(), "hello ");
  std::string str = "tnet";
  ls << str;
  EXPECT_EQ(getString(), "hello tnet");
  ls.resetBuffer();
}

TEST_F(LogStreamTest, FmtTest) {
  ls << tnet::Fmt("%4d", 1);
  EXPECT_EQ(getString(), "   1");
  ls.resetBuffer();
  ls << tnet::Fmt("%4.2f", 1.2);
  EXPECT_EQ(getString(), "1.20");
  ls.resetBuffer();
  ls << tnet::Fmt("%4.2f", 1.2) << tnet::Fmt("%4d", 42);
  EXPECT_EQ(getString(), "1.20  42");
  ls.resetBuffer();
}

TEST_F(LogStreamTest, SizeTest) {
  for (int i = 0; i < 399; i++) {
    ls << "1234567890";
    EXPECT_EQ(getLength(), 10 * (i + 1));
    EXPECT_EQ(getAvail(), 4000 - 10 * (i + 1));
  }
  ls << "1234567890";
  EXPECT_EQ(getLength(), 3990);
  EXPECT_EQ(getAvail(), 10);
  ls << "123456789";
  EXPECT_EQ(getLength(), 3999);
  EXPECT_EQ(getAvail(), 1);
  ls.resetBuffer();
}

#ifndef GTEST_ALL

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

#endif  // GTEST_ALL
