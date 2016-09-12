#include <gtest/gtest.h>
#include <tnet/base/StringPiece.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

class StringPieceTest : public testing::Test {
 protected:
  virtual void SetUp() {
    testStringStd = std::string(testString);
    sps.emplace_back();                       // 0
    sps.emplace_back(testStringStd);          // 1
    sps.emplace_back(testString);             // 2
    sps.emplace_back(offset, lenTestString);  // 3
    sps.emplace_back(compString);             // 4
    sps.emplace_back(startWithS);             // 5
  }
  virtual void TearDown() {}
  const char* testString = "teststringteststring";
  const char* compString = "helloworld";
  const char* startWithS = "teststring";
  const char* offset = testString;
  std::string testStringStd;
  int         lenTestString = 20;
  std::vector<tnet::StringPiece> sps;
};

TEST_F(StringPieceTest, StaticTest) {
  EXPECT_EQ(sps[0].data(), nullptr);
  EXPECT_EQ(sps[1].data(), testStringStd.data());
  for (int i = 2; i < 4; i++) {
    EXPECT_EQ(sps[i].data(), testString);
  }

  EXPECT_EQ(sps[0].size(), 0);
  EXPECT_TRUE(sps[0].empty());
  EXPECT_EQ(sps[0].begin(), nullptr);
  EXPECT_EQ(sps[0].end(), nullptr);
  EXPECT_FALSE(sps[0] == sps[1]);
  EXPECT_TRUE(sps[0] != sps[1]);
  for (int i = 1; i < 4; i++) {
    EXPECT_EQ(sps[i].size(), lenTestString);
    EXPECT_FALSE(sps[i].empty());
    EXPECT_EQ(*(sps[i].begin()), testString[0]);
    EXPECT_EQ(sps[i].begin(), sps[i].data());
    EXPECT_EQ(*(sps[i].end() - 1), testString[lenTestString - 1]);
    EXPECT_EQ(sps[i].end(), sps[i].begin() + sps[i].size());
    for (int j = 0; j < 20; j++) {
      EXPECT_EQ(sps[i][j], testString[j]);
    }
    EXPECT_FALSE(sps[i] == sps[4]);
    EXPECT_TRUE(sps[i] == sps[1]);
    EXPECT_TRUE(sps[i] != sps[4]);
    EXPECT_FALSE(sps[i] != sps[1]);
    EXPECT_NE(sps[i].compare(sps[4]), 0);
    EXPECT_EQ(sps[i].compare(sps[1]), 0);
    EXPECT_EQ(sps[i].std_string(), testStringStd);
    EXPECT_TRUE(sps[i].starts_with(sps[5]));
    EXPECT_FALSE(sps[i].starts_with(sps[4]));
    std::string copyStringStd;
    sps[i].copyToStdString(&copyStringStd);
    EXPECT_EQ(copyStringStd, testStringStd);
  }
}

TEST_F(StringPieceTest, DynamicTest) {
  for (int i = 1; i < 4; i++) {
    tnet::StringPiece temp = sps[i];
    sps[i].clear();
    EXPECT_TRUE(sps[i] == sps[0]);
    sps[i].set(testString);
    EXPECT_EQ(sps[i], temp);
    sps[i].clear();
    sps[i].set(offset, lenTestString);
    EXPECT_EQ(sps[i], temp);
  }
}

TEST_F(StringPieceTest, RemoveTest) {
  for (int i = 1; i < 4; i++) {
    sps[i].remove_prefix(2);
    EXPECT_EQ(*(sps[i].begin()), testString[2]);
    EXPECT_EQ(*(sps[i].end() - 1), testString[lenTestString - 1]);
    sps[i].remove_prefix(4);
    EXPECT_EQ(*(sps[i].begin()), testString[6]);
    EXPECT_EQ(*(sps[i].end() - 1), testString[lenTestString - 1]);
    sps[i].remove_suffix(3);
    EXPECT_EQ(*(sps[i].end() - 1), testString[lenTestString - 3 - 1]);
  }
}

#ifndef GTEST_ALL

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

#endif  // GTEST_ALL
