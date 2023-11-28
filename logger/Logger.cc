#include <string.h>
#include <unistd.h>
#include <webserver/logger/Logger.h>

#include <chrono>
#include <map>

using namespace webserver;

const std::map<Logger::LogLevelEnum, const char*> LogLevelEnumString = {
    {Logger::LogLevelEnum::DEBUG, "DEBUG"},
    {Logger::LogLevelEnum::INFO, "INFO"},
    {Logger::LogLevelEnum::WARN, "WARN"},
    {Logger::LogLevelEnum::ERROR, "ERROR"},
    {Logger::LogLevelEnum::FATAL, "FATAL"},
};

Logger::OutputFunc Logger::kOutputFunc = [](const char* data, int len) {
  fwrite(data, 1, len, stdout);
};
Logger::FlushFunc Logger::kFlushFunc = []() { fflush(stdout); };
Logger::LogLevelEnum Logger::kLogLevel = Logger::LogLevelEnum::DEBUG;

Logger::Logger(std::string filename, int line, std::string func,
               LogLevelEnum level)
    : basename_(filename.substr(filename.rfind('/') + 1)),
      line_(line),
      func_(func),
      buffer_(),
      level_(level) {
  std::chrono::system_clock c;
  auto now = std::chrono::system_clock::now();
  time_t timestamp = std::chrono::system_clock::to_time_t(now);
  std::tm* tm_info = std::localtime(&timestamp);
  // get a string about year mounth day hour second minite second
  std::string s;
  s += std::to_string(tm_info->tm_year + 1900);
  s += "-";
  s += std::to_string(tm_info->tm_mon + 1);
  s += "-";
  s += std::to_string(tm_info->tm_mday);
  s += " ";
  s += std::to_string(tm_info->tm_hour);
  s += ":";
  s += std::to_string(tm_info->tm_min);
  s += ":";
  s += std::to_string(tm_info->tm_sec);
  s += " ";
  s += std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(
                          now.time_since_epoch())
                          .count() %
                      1000000);
  s += " ";
  s.append(LogLevelEnumString.at(level));
  s += " - ";
  s.append(basename_);
  s.append(":");
  s.append(std::to_string(line_));
  s.append(" ");
  s.append(func_);
  s.append("\t- ");
  buffer_.append(s);
}

Logger::~Logger() {
  buffer_.append("\n", 1);
  Buffer buffer(std::move(buffer_));
  kOutputFunc(buffer.begin_read(), buffer.readable_bytes());
  kFlushFunc();
  if (level_ == LogLevelEnum::FATAL) {
    ::abort();
  }
}

Logger::LogLevelEnum Logger::get_log_level() { return kLogLevel; }
void Logger::set_log_level(LogLevelEnum level) { kLogLevel = level; }
void Logger::set_output(OutputFunc out) { kOutputFunc = out; }
void Logger::set_flush(FlushFunc flush) { kFlushFunc = flush; }

// self& operator<<(short);
// self& operator<<(unsigned short);
// self& operator<<(int);
// self& operator<<(unsigned int);
// self& operator<<(long);
// self& operator<<(unsigned long);
// self& operator<<(long long);
// self& operator<<(unsigned long long);
// self& operator<<(float);
// self& operator<<(double);
// self& operator<<(long double);
// self& operator<<(char);
// self& operator<<(const char*);
// self& operator<<(const unsigned char*);
// self& operator<<(const std::string&);
// self& operator<<(const void*);
// self& operator<<(bool);

Logger& Logger::operator<<(short num) {
  buffer_.append(std::to_string(num));
  return *this;
}
Logger& Logger::operator<<(unsigned short num) {
  buffer_.append(std::to_string(num));
  return *this;
}
Logger& Logger::operator<<(int num) {
  buffer_.append(std::to_string(num));
  return *this;
}
Logger& Logger::operator<<(unsigned int num) {
  buffer_.append(std::to_string(num));
  return *this;
}
Logger& Logger::operator<<(long num) {
  buffer_.append(std::to_string(num));
  return *this;
}
Logger& Logger::operator<<(unsigned long num) {
  buffer_.append(std::to_string(num));
  return *this;
}
Logger& Logger::operator<<(long long num) {
  buffer_.append(std::to_string(num));
  return *this;
}
Logger& Logger::operator<<(unsigned long long num) {
  buffer_.append(std::to_string(num));
  return *this;
}
Logger& Logger::operator<<(float num) {
  buffer_.append(std::to_string(num));
  return *this;
}
Logger& Logger::operator<<(double num) {
  buffer_.append(std::to_string(num));
  return *this;
}
Logger& Logger::operator<<(long double num) {
  buffer_.append(std::to_string(num));
  return *this;
}
Logger& Logger::operator<<(char c) {
  buffer_.append(&c, 1);
  return *this;
}
Logger& Logger::operator<<(const char* str) {
  if (str) {
    buffer_.append(str, strlen(str));
  } else {
    buffer_.append("(null)", 6);
  }
  return *this;
}
Logger& Logger::operator<<(const unsigned char* str) {
  return operator<<(reinterpret_cast<const char*>(str));
}
Logger& Logger::operator<<(const std::string& str) {
  buffer_.append(str.c_str(), str.size());
  return *this;
}
Logger& Logger::operator<<(const void* p) {
  std::uintptr_t num = reinterpret_cast<std::uintptr_t>(p);
  if (num == 0) {
    buffer_.append("nullptr", 7);
  } else {
    buffer_.append("0x", 2);
    buffer_.append(std::to_string(num));
  }
  return *this;
}
Logger& Logger::operator<<(bool b) {
  if (b) {
    buffer_.append("true", 4);
  } else {
    buffer_.append("false", 5);
  }
  return *this;
}