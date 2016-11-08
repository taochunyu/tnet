#include <gtest/gtest.h>
#include <tnet/base/FileUtil.h>
#include <errno.h>
#include <cinttypes>
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <string>
using namespace tnet;

TEST(FileUtilTest, OnlyInOSX) {
  std::string result;
  char buffer[32];
  int64_t size = 0;
  int err;
  result = "";
  err = FileUtil::readFile("/proc/self", 1024, &result, &size);
  sprintf(buffer, "%d %zd %" PRIu64 "\n", err, result.size(), size);
  EXPECT_STREQ(buffer, "2 0 0\n");
  result = "";
  err = FileUtil::readFile("/proc/self", 1024, &result, NULL);
  sprintf(buffer, "%d %zd %" PRIu64 "\n", err, result.size(), size);
  EXPECT_STREQ(buffer, "2 0 0\n");
  result = "";
  err = FileUtil::readFile("/proc/self/cmdline", 1024, &result, &size);
  sprintf(buffer, "%d %zd %" PRIu64 "\n", err, result.size(), size);
  EXPECT_STREQ(buffer, "2 0 0\n");
  result = "";
  err = FileUtil::readFile("/dev/null", 1024, &result, &size);
  sprintf(buffer, "%d %zd %" PRIu64 "\n", err, result.size(), size);
  EXPECT_STREQ(buffer, "0 0 0\n");
  result = "";
  err = FileUtil::readFile("/dev/zero", 1024, &result, &size);
  sprintf(buffer, "%d %zd %" PRIu64 "\n", err, result.size(), size);
  EXPECT_STREQ(buffer, "0 1024 0\n");
  result = "";
  err = FileUtil::readFile("/notexist", 1024, &result, &size);
  sprintf(buffer, "%d %zd %" PRIu64 "\n", err, result.size(), size);
  EXPECT_STREQ(buffer, "2 0 0\n");
  result = "";
  err = FileUtil::readFile("/dev/zero", 102400, &result, &size);
  sprintf(buffer, "%d %zd %" PRIu64 "\n", err, result.size(), size);
  EXPECT_STREQ(buffer, "0 102400 0\n");
  result = "";
  err = FileUtil::readFile("/dev/zero", 102400, &result, NULL);
  sprintf(buffer, "%d %zd %" PRIu64 "\n", err, result.size(), size);
  EXPECT_STREQ(buffer, "0 102400 0\n");
}
/*
TEST(FileUtilTest, OnlyInCentos7) {
  std::string result;
  char buffer[32];
  int64_t size = 0;
  int err;
  result = "";
  err = FileUtil::readFile("/proc/self", 1024, &result, &size);
  sprintf(buffer, "%d %zd %" PRIu64 "\n", err, result.size(), size);
  EXPECT_STREQ(buffer, "21 0 0\n");
  result = "";
  err = FileUtil::readFile("/proc/self", 1024, &result, NULL);
  sprintf(buffer, "%d %zd %" PRIu64 "\n", err, result.size(), size);
  EXPECT_STREQ(buffer, "21 0 0\n");
  result = "";
  err = FileUtil::readFile("/proc/self/cmdline", 1024, &result, &size);
  sprintf(buffer, "%d %zd %" PRIu64 "\n", err, result.size(), size);
  EXPECT_STREQ(buffer, "0 8 0\n");
  result = "";
  err = FileUtil::readFile("/dev/null", 1024, &result, &size);
  sprintf(buffer, "%d %zd %" PRIu64 "\n", err, result.size(), size);
  EXPECT_STREQ(buffer, "0 0 0\n");
  result = "";
  err = FileUtil::readFile("/dev/zero", 1024, &result, &size);
  sprintf(buffer, "%d %zd %" PRIu64 "\n", err, result.size(), size);
  EXPECT_STREQ(buffer, "0 1024 0\n");
  result = "";
  err = FileUtil::readFile("/notexist", 1024, &result, &size);
  sprintf(buffer, "%d %zd %" PRIu64 "\n", err, result.size(), size);
  EXPECT_STREQ(buffer, "2 0 0\n");
  result = "";
  err = FileUtil::readFile("/dev/zero", 102400, &result, &size);
  sprintf(buffer, "%d %zd %" PRIu64 "\n", err, result.size(), size);
  EXPECT_STREQ(buffer, "0 102400 0\n");
  result = "";
  err = FileUtil::readFile("/dev/zero", 102400, &result, NULL);
  sprintf(buffer, "%d %zd %" PRIu64 "\n", err, result.size(), size);
  EXPECT_STREQ(buffer, "0 102400 0\n");
}
*/

#ifndef GTEST_ALL

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

#endif  // GTEST_ALL
