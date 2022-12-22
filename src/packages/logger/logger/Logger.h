#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>

class LogType {
  public:
    explicit LogType(int value) : value_(value) {}

    bool operator<(const LogType& other) const { return value_ < other.value_; }
    bool operator<=(const LogType& other) const { return value_ <= other.value_; }
    bool operator>(const LogType& other) const { return value_ > other.value_; }
    bool operator>=(const LogType& other) const { return value_ >= other.value_; }
    bool operator==(const LogType& other) const { return value_ == other.value_; }

  private:
    int value_;
};

const LogType LOG_INFO(0);
const LogType LOG_WARNING(1);
const LogType LOG_ERROR(2);

class LogEntry {
  public:
    explicit LogEntry(const LogType& type, const std::string& message) : type_(type), message_(message) {}

    std::string ToString() const;
    std::string ToStringWithoutTimestamp() const;
    std::string ToColoredString(const std::string& color) const;
    std::string ToColoredStringWithoutTimestamp(const std::string& color) const;

  private:
    LogType type_;
    std::string message_;
};

class Logger {
  public:
    explicit Logger(std::ostream& stream) : stream_(stream) {}

    void SetLogLevel(const LogType& logLevel);
    void Log(const LogType& type, const std::string& message);
    void LogInfo(const std::string& message);
    void LogWarning(const std::string& message);
    void LogError(const std::string& message);

  private:
    std::string GetColorForLogType(const LogType& type);

    std::vector<LogEntry> messages_;
    std::ostream& stream_;
    LogType logLevel_ = LOG_INFO;
};
