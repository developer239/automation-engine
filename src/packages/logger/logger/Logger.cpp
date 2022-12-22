#include "Logger.h"

std::string LogEntry::ToString() const {
  std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::string output(20, '\0');
  std::strftime(&output[0], output.size(), "%d-%b-%Y %H:%M:%S", std::localtime(&now));
  return "[" + output + "]: " + message_;
}

std::string LogEntry::ToStringWithoutTimestamp() const { return message_; }

std::string LogEntry::ToColoredString(const std::string& color) const {
  std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::string output(20, '\0');
  std::strftime(&output[0], output.size(), "%d-%b-%Y %H:%M:%S", std::localtime(&now));
  return color + "[" + output + "]: " + message_ + "\033[0m";
}

std::string LogEntry::ToColoredStringWithoutTimestamp(const std::string& color) const {
  return color + message_ + "\033[0m";
}

void Logger::SetLogLevel(const LogType& logLevel) { logLevel_ = logLevel; }

void Logger::Log(const LogType& type, const std::string& message) {
  if (type < logLevel_) {
    return;
  }
  LogEntry logEntry(type, message);
  messages_.push_back(logEntry);
  stream_ << logEntry.ToColoredString(GetColorForLogType(type)) << std::endl;
}

std::string Logger::GetColorForLogType(const LogType& type) {
  if (type >= LOG_ERROR) {
    return "\x1B[91m";
  } else if (type >= LOG_WARNING) {
    return "\x1B[93m";
  } else {
    return "\x1B[32m";
  }
}

void Logger::LogInfo(const std::string& message) {
  Log(LOG_INFO, message);
}

void Logger::LogWarning(const std::string& message) {
  Log(LOG_WARNING, message);
}

void Logger::LogError(const std::string& message) {
  Log(LOG_ERROR, message);
}
