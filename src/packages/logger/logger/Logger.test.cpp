#include "Logger.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <sstream>

TEST(LogTypeTest, ComparisonOperators) {
  const LogType info(0);
  const LogType warning(1);
  const LogType error(2);

  EXPECT_TRUE(info < warning);
  EXPECT_FALSE(warning < info);

  EXPECT_TRUE(info <= warning);
  EXPECT_TRUE(info <= info);
  EXPECT_FALSE(warning <= info);

  EXPECT_FALSE(info > warning);
  EXPECT_TRUE(error > warning);

  EXPECT_FALSE(info >= warning);
  EXPECT_TRUE(warning >= info);
  EXPECT_TRUE(info >= info);

  EXPECT_TRUE(info == info);
  EXPECT_FALSE(info == warning);
}

class LoggerTest : public testing::Test {
  protected:
    LoggerTest() : logger_(output_stream_) {}

    std::stringstream output_stream_;
    Logger logger_;
};

TEST_F(LoggerTest, Log) {
  logger_.LogError("error message");

  std::string output;
  std::getline(output_stream_, output, '\n');
  EXPECT_THAT(output, testing::HasSubstr("error message"));
  EXPECT_THAT(output, testing::HasSubstr("\x1B[91m"));
}

TEST_F(LoggerTest, SetLogLevel) {
  logger_.SetLogLevel(LOG_WARNING);

  logger_.LogError("error message");
  EXPECT_THAT(output_stream_.str(), testing::HasSubstr("error message"));
  EXPECT_THAT(output_stream_.str(), testing::HasSubstr("\x1B[91m"));
  output_stream_.str(std::string());

  logger_.LogWarning("warning message");
  EXPECT_THAT(output_stream_.str(), testing::HasSubstr("warning message"));
  EXPECT_THAT(output_stream_.str(), testing::HasSubstr("\x1B[93m"));
  output_stream_.str(std::string());

  logger_.LogInfo("info message");
  EXPECT_THAT(output_stream_.str(), testing::Not(testing::HasSubstr("info message")));
  EXPECT_THAT(output_stream_.str(), testing::Not(testing::HasSubstr("\x1B[32m")));
}
